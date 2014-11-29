/*
 * ami_tool.c
 *
 * ** ami connection states **
 * DISCONNECTED		--[connect]-->				CONNECTED
 *
 * CONNECTED		--[login failed]-->			DISCONNECTED
 * CONNECTED		--[event DISCONNECTED]-->	DISCONNECTED
 * CONNECTED		--[event FULLYBOOTED]-->	LOGGED_IN
 *
 * LOGGED_IN		--[BRCMPortsShow resp]-->	READY
 * LOGGED_IN		--[event DISCONNECTED]-->	DISCONNECTED
 *
 * READY			--[event DISCONNECTED]-->	DISCONNECTED
 * READY			--[event BRCM MODULE UL]-->	LOGGED_IN
 *
 * - Workaround for the fact that ports in software on DG201 are enumerated opposite to LED and physical ports has been removed,
 *   since we don't know what hardware we are running on.
 */

#include "ami_tool.h"
#include <signal.h>
#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include "ami_connection.h"
#include "ucix.h"

static bool running = false;

//TODO: all uci things here
void ucix_reload();
static struct uci_context *uci_ctx = NULL;

//TODO: all ubus things here
static bool ubus_connected = false;
static struct ubus_context *ctx = NULL;
static struct blob_buf bb;
static struct blob_buf b_led;

static SIP_PEER sip_peers[SIP_ACCOUNT_UNKNOWN + 1];

struct codec {
  char *key;
  char *value;
  unsigned int bitrate;
  struct codec *next;
};
static struct codec *codec_create();
static void codec_delete(struct codec *codec);
static void codec_cb(const char * name, void *codec);

static int rtpstart_current = 0;
static int rtpend_current = 0;
static IP* ip_list_current = NULL;
static int ip_list_length_current = 0;

const char *portno = "5038"; //AMI port number
char hostname[] = "127.0.0.1"; //AMI hostname
char username[128] = "local"; //AMI username
char password[128] = "local"; //AMI password

static int ubus_send_brcm_event(const PORT_MAP *port, const char *key, const char *value);
static int ubus_send_sip_event(const SIP_PEER *peer, const char *key, const int value);

/* Asterisk states */
void set_state(AMI_STATE state, ami_connection* con);
static int voice_led_count = 1; //Number of voice leds on board
static int fxs_line_count = 0; //Number of FXS ports on board
static int dect_line_count = 0; //Number of DECT ports on board
static Led* led_config = NULL; //Array of led configs (one for each led)

/* Forward declaration of AMI functions and callbacks */
void ami_handle_event(ami_connection* con, ami_event event);
void handle_registry_event(ami_event event);
void handle_brcm_event(ami_connection* con, ami_event event);
void handle_varset_event(ami_event event);
void handle_registry_entry_event(ami_event event);
void on_login_response(ami_connection* con, char* buf);
void on_brcm_module_show_response(ami_connection* con, char* buf);
void on_brcm_ports_show_response(ami_connection* con, char* buf);


/* For debug. Log all resolved domain names for SIP peer */
void sip_peer_list_ip(SIP_PEER *peer)
{
	int i;
	for (i=0; i<peer->ip_list_length; i++) {
		IP ip = peer->ip_list[i];
		printf("%d %s\n", ip.family, ip.addr);
	}
}

/* Add IP to list for SIP peer */
void sip_peer_add_ip(SIP_PEER *peer, char *addr, int family) {
	int i;

	if (peer->ip_list_length >= MAX_IP_LIST_LENGTH) {
		fprintf(stderr, "Could not add IP %s to peer %s, ip list is full\n", addr, peer->account.name);
		return;
	}

	for (i=0; i < peer->ip_list_length; i++) {
		IP ip = peer->ip_list[i];
		if (family == ip.family && strcmp(addr, ip.addr) == 0) {
			return;
		}
	}
	strcpy(peer->ip_list[peer->ip_list_length].addr, addr);
	peer->ip_list[peer->ip_list_length].family = family;
	peer->ip_list_length++;
}

/*
 * Determine how many voice leds are present in current hardware config
 */
int uci_get_voice_led_count()
{
	/* Initialize */
	struct uci_context *hw_uci_ctx = ucix_init_path("/lib/db/config/", "hw");
	if(!hw_uci_ctx) {
		printf("Failed to get uci context for path /lib/db/config\n");
		return 1; //Assume a single voice led
	}

	int led_count = ucix_get_option_int(hw_uci_ctx, "hw", "board", "VoiceLeds", 1);
	printf("Found %i voice leds\n", led_count);
	ucix_cleanup(hw_uci_ctx);
	return led_count;
}

/*
 * Get the called_lines configuration for a sip peer
 */
const char* uci_get_called_lines(const SIP_PEER* peer)
{
	ucix_reload();
	return ucix_get_option(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "call_lines");
}

int uci_get_rtp_port_start()
{
	ucix_reload();
	return ucix_get_option_int(uci_ctx, UCI_VOICE_PACKAGE, "SIP", "rtpstart", RTP_RANGE_START_DEFAULT);
}

int uci_get_rtp_port_end()
{
	ucix_reload();
	return ucix_get_option_int(uci_ctx, UCI_VOICE_PACKAGE, "SIP", "rtpend", RTP_RANGE_END_DEFAULT);
}

int uci_get_sip_proxy(struct list_head *proxies)
{
	ucix_reload();
	return ucix_get_option_list(uci_ctx, UCI_VOICE_PACKAGE, "SIP", "sip_proxy", proxies);
}

const char* uci_get_peer_host(SIP_PEER *peer)
{
	ucix_reload();
	int enabled = ucix_get_option_int(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "enabled", 0);
	if (enabled == 0) {
		return NULL;
	}
	return ucix_get_option(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "host");
}

const char* uci_get_peer_domain(SIP_PEER *peer)
{
	ucix_reload();
	int enabled = ucix_get_option_int(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "enabled", 0);
	if (enabled == 0) {
		return NULL;
	}
	return ucix_get_option(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "domain");
}

int uci_get_peer_enabled(SIP_PEER* peer)
{
	ucix_reload();
	return ucix_get_option_int(uci_ctx, UCI_VOICE_PACKAGE, peer->account.name, "enabled", 0);
}

struct codec *uci_get_codecs()
{
	/* Create space for first codec */
	struct codec *c = codec_create();

	ucix_reload();
	ucix_for_each_section_type(uci_ctx, UCI_VOICE_PACKAGE, "supported_codec", codec_cb, c);
	return c;
}

/* Resolv name into ip (A or AAA record), update IP list for peer */
static int resolv(SIP_PEER *peer, const char *domain)
{
	struct addrinfo *result;
	struct addrinfo *res;
	int error;

	/* Resolve the domain name into a list of addresses, don't specify any services */
	error = getaddrinfo(domain, NULL, NULL, &result);
	if (error != 0)
	{
		printf("error in getaddrinfo: %s\n", gai_strerror(error));
		return 1;
	}

	/* Loop over all returned results and convert IP from network to textual form */
	for (res = result; res != NULL; res = res->ai_next)
	{
		char ip_addr[NI_MAXHOST];
		void *in_addr;
		switch (res->ai_family) {
			case AF_INET: {
				struct sockaddr_in *s_addr = (struct sockaddr_in *) res->ai_addr;
				in_addr = &s_addr->sin_addr;	
				break;
			}
#ifdef USE_IPV6
			case AF_INET6: {
				struct sockaddr_in6 *s_addr6 = (struct sockaddr_in6 *) res->ai_addr;
				in_addr = &s_addr6->sin6_addr;
				break;
			}
#endif
			default:
				continue;
		}
		inet_ntop(res->ai_family, in_addr, (void *)&ip_addr, NI_MAXHOST);

		/* Add to list of IPs if not already there */
		sip_peer_add_ip(peer, ip_addr, res->ai_family);
	}   

	freeaddrinfo(result);

	return 0;

}

/* Compare two struct IP */
int ip_cmp(const void *a, const void *b) 
{ 
	const IP *ia = (const IP *)a;
	const IP *ib = (const IP *)b;
	if (ia->family < ib->family) {
		return -1;
	}
	if (ib->family < ia->family) {
		return 1;
	}
	return strcmp(ia->addr, ib->addr);
} 

/* Create a set of all resolved IPs for all peers */
IP* create_ip_set(int family, int *ip_list_length) {
	SIP_PEER *peer;
	IP *ip_list;

	*ip_list_length = 0;
	ip_list = (IP *) malloc(MAX_IP_LIST_LENGTH * sizeof(struct IP));

	/* This is O(n^3) but the lists are small... */
	peer = sip_peers;
	while (peer->account.id != SIP_ACCOUNT_UNKNOWN) {
		int i;
		for (i=0; i<peer->ip_list_length; i++) {
			int add = 1;
			int j;

			if (peer->ip_list[i].family != family) {
				continue;
			}

			for (j=0; j<*ip_list_length; j++) {
				if (ip_list[j].family == peer->ip_list[i].family && 
					strcmp(ip_list[j].addr, peer->ip_list[i].addr) == 0) {
					/* IP alread in set */
					add = 0;
					break;
				}
			}
			if (add) {
				/* IP not found in set */
				strcpy(ip_list[*ip_list_length].addr, peer->ip_list[i].addr);
				ip_list[*ip_list_length].family = peer->ip_list[i].family;
				(*ip_list_length)++;
				if (*ip_list_length == MAX_IP_LIST_LENGTH) {
					/* ip_list is full */
					return ip_list;
				}
			}
		}
		peer++;
	}

	return ip_list;
}

/* Compare two IP sets */
int compare_ip_set(IP* ip_list1, int ip_list_length1, IP* ip_list2, int ip_list_length2)
{
	if (ip_list1 == NULL && ip_list2 == NULL) {
		return 0;
	}

	if (ip_list1 == NULL) {
		return -1;
	}

	if (ip_list2 == NULL) {
		return 1;
	}

	if (ip_list_length1 < ip_list_length2) {
		return -1;
	}

	if (ip_list_length2 < ip_list_length1) {
		return 1;
	}

	int i;
	for(i=0; i<ip_list_length1; i++) {
		int rv = strcmp(ip_list1[i].addr, ip_list2[i].addr);
		if (rv) {
			return rv;
		}
	}
	return 0;
}

void write_firewall(int family)
{
	char *tables_file;
	char *iptables_bin;
	char buf[BUFLEN];
	int ip_list_length;
	IP* ip_list;

	/* Is there a change in IP or RTP port range? */
	ip_list = create_ip_set(family, &ip_list_length);
	int rtpstart = uci_get_rtp_port_start();
	int rtpend = uci_get_rtp_port_end();

	if (compare_ip_set(ip_list_current, ip_list_length_current, ip_list, ip_list_length) == 0 &&
	    rtpstart_current == rtpstart &&
	    rtpend_current == rtpend) {
		printf("No changes in IP or RTP port range\n");
		free(ip_list);
		return;	
	}

	/* Clear old firewall settings, write timestamp */
	time_t rawtime;
	struct tm * timeinfo;
	char timebuf[BUFLEN];
	time(&rawtime);
	timeinfo = (struct tm*) localtime(&rawtime);
	strftime(timebuf, BUFLEN, "%Y-%m-%d %H:%M:%S", timeinfo);

	tables_file = IPTABLES_FILE;
	iptables_bin = IPTABLES_BIN;
#ifdef USE_IPV6
	if (family == AF_INET6) {
		iptables_bin = IP6TABLES_BIN;
		tables_file = IP6TABLES_FILE;
	}
#endif
	snprintf((char *)&buf, BUFLEN, "%s \"# Created by ami_tool %s\" > %s",
		ECHO_BIN,
		timebuf,
		tables_file);
	printf("%s\n", buf);
	system(buf);

	/* Create an iptables rule for each IP in set */
	int i;
	for (i=0; i<ip_list_length; i++) {
		snprintf((char *)&buf, BUFLEN, "%s \"%s -I %s -s %s -j ACCEPT\" >> %s",
			ECHO_BIN,
			iptables_bin,
			IPTABLES_CHAIN,
			ip_list[i].addr,
			tables_file);
		printf("%s\n", buf);
		system(buf);
	}
	if (ip_list_current) {
		free(ip_list_current);
	}
	ip_list_current = ip_list;
	ip_list_length_current = ip_list_length;

	/* Open up for RTP traffic */
	snprintf((char *)&buf, BUFLEN, "%s \"%s -I %s -p udp --dport %d:%d -j ACCEPT\" >> %s",
		ECHO_BIN,
		iptables_bin,
		IPTABLES_CHAIN,
		rtpstart,
		rtpend,
		tables_file);
	printf("%s\n", buf);
	system(buf);
	rtpstart_current = rtpstart;
	rtpend_current = rtpend;

	snprintf((char *)&buf, BUFLEN, "/etc/init.d/firewall reload");
	printf("%s\n", buf);
	system(buf);
}

/* Resolv host and add IPs to iptables */
int handle_iptables(SIP_PEER *peer, int doResolv)
{
	/* Clear old IP list */
	peer->ip_list_length = 0;

	if (doResolv) {
		/* Get domain to resolv */
		const char* domain = uci_get_peer_domain(peer);
		if (domain) {
			resolv(peer, domain);
		}
		else {
			printf("Failed to get sip domain\n");
			return 1;
		}

		const char* host = uci_get_peer_host(peer);
		if (host) {
			resolv(peer, host);
		}

		/* Get sip proxies and resolv if configured */
		struct ucilist proxies;
		INIT_LIST_HEAD(&proxies.list);
		if (!uci_get_sip_proxy(&proxies.list)) {
			struct list_head *i;
			struct list_head *tmp;
			list_for_each_safe(i, tmp, &proxies.list)
			{
				struct ucilist *proxy = list_entry(i, struct ucilist, list);
				resolv(peer, proxy->val);
				free(proxy->val);
				free(proxy);
			}
		}
	}

	/* Write new config to firewall.sip and reload firewall */
	printf("\nIPv4:\n");
	write_firewall(AF_INET);
#ifdef USE_IPV6
	printf("\nIPv6:\n");
	write_firewall(AF_INET6);
#endif
	printf("\n");

	return 0;
}

void log_sip_peers()
{
	const SIP_PEER *peers = sip_peers;
	while (peers->account.id != SIP_ACCOUNT_UNKNOWN) {
		printf("sip_peer %d:\n", peers->account.id);
		printf("\tname %s:\n", peers->account.name);
		printf("\tsip_registry_request_sent: %d\n", peers->sip_registry_request_sent);
		printf("\tsip_registry_registered: %d\n", peers->sip_registry_registered);
		printf("\n");
		peers++;
	}
}

static int brcm_subchannel_active(const PORT_MAP *port) {
	int subchannel_id;
	for (subchannel_id=0; subchannel_id<2; subchannel_id++) {
		if (strcmp(port->sub[subchannel_id].state, "ONHOOK") && strcmp(port->sub[subchannel_id].state, "CALLENDED")) {
			return 1;
		}
		return 0;
	}

	return 0;
}

/**********************************
 * LED management
 **********************************/

void manage_leds() {

	if (state != READY) {
		manage_led(LN_VOICE1, LS_ERROR);
		manage_led(LN_VOICE2, LS_ERROR);
		return;
	}

	int i;
	for (i = 0; i < voice_led_count; i++) {
		Led* led = &led_config[i];
		LED_STATE new_state = get_led_state(led);
		if (new_state != led->state) {
			manage_led(led->name, new_state);
		}
		led->state = new_state;
	}
}

void manage_led(LED_NAME led, LED_STATE state) {
	const LED_NAME_MAP *leds = led_names;
	const LED_STATE_MAP *states = led_states;
	LED_CURRENT_STATE_MAP *current_state = led_current_states;

	//Check and set current state
	while (current_state->name != LN_UNKNOWN) {
		if (current_state->name == led) {
			if (current_state->state == state) {
				//No need to update led
				return;
			}
			current_state->state = state;
			break;
		}
		current_state++;
	}

	//Lookup led name
	while (leds->name != led) {
		leds++;
		if (leds->name == LN_UNKNOWN) {
			printf("Unknown led name\n");
			return;
		}
	}

	//Lookup led state
	while (states->state != state) {
		states++;
		if (states->state == LS_UNKNOWN) {
			printf("Unknown led state\n");
			return;
		}
	}

	//Lookup the id of led object
	uint32_t id;
	if (ubus_lookup_id(ctx, leds->str, &id)) {
		fprintf(stderr, "Failed to look up %s object\n", leds->str);
		return;
	}

	//Specify the state we want to set
	blob_buf_init(&b_led, 0);
	blobmsg_add_string(&b_led, "state", states->str);

	//Invoke state change
	printf("Setting LED %s state to %s\n", leds->str, states->str);
	ubus_invoke(ctx, id, "set", b_led.head, NULL, 0, 1000);
}

/*
 * Calculate a new state for a Led, based on the state of governing lines and accounts.
 */
LED_STATE get_led_state(Led* led)
{
	//If one of the governing lines are active, led should be in notice mode
	int i;
	for(i = 0; i < led->num_ports; i++) {
		if (brcm_subchannel_active(led->ports[i])) {
			//printf("LED %d, PORT %s is active => LS_NOTICE\n", led->name, led->ports[i]->name);
			return LS_NOTICE;
		}
	}

	//Check state of governing accounts
	LED_STATE tmp = LS_OFF;
	for(i = 0; i < led->num_peers; i++) {
		SIP_PEER* peer = led->peers[i];
		if (!peer->sip_registry_registered) {
			//printf("LED %d: PEER is not registered => LS_ERROR\n", led->name);
			return LS_ERROR;
		}
		else {
			//printf("LED %d: PEER is registered => LS_OK\n", led->name);
			tmp = LS_OK;
		}
	}
	return tmp;
}

void free_led_config()
{
	int i;
	for (i = 0; i < voice_led_count; i++) {
		Led* led = &led_config[i];
		free(led->peers);
		free(led->ports);
	}
	free(led_config);
	led_config = NULL;
}

/*
 * configure_leds
 *
 * Configure which lines and peers that should determine the state of the
 * voice led(s). The configuration is then used by manage_leds.
 *
 * led_config is rebuilt whenever a new line config is read from chan_brcm
 */
void configure_leds()
{
	if (state != READY) {
		return; //No need to configure leds yet
	}

	if (led_config) {
		free_led_config();
	}

	led_config = calloc(voice_led_count, sizeof(Led));
	int i;

	if (voice_led_count == 1) {
		/*
		 * Single LED - all ports govern status
		 */
		printf("Single LED configuration\n");

		PORT_MAP** all_ports = calloc(dect_line_count + fxs_line_count, sizeof(PORT_MAP*));
		for (i = 0; i < (dect_line_count + fxs_line_count); i++) {
			all_ports[i] = &brcm_ports[i];
		}
		led_config[0].state = LS_UNKNOWN;
		led_config[0].name = LN_VOICE1;
		led_config[0].ports = all_ports;
		led_config[0].num_ports = dect_line_count + fxs_line_count;
	}
	else if (voice_led_count > 1) {
		/*
		 * Two LEDs, make best use of them!
		 * (Assume two leds, if there are more, we currently do not use them)
		 */
		if (dect_line_count > 0) {
			/*
			 * LED1  = FXS, LED2 = DECT
			 * dects are lower numbered, fxs higher
			 */
			printf("Dual LED configuration, FXS and DECT\n");
			PORT_MAP** dect_ports = calloc(dect_line_count, sizeof(PORT_MAP*));
			for (i = 0; i < dect_line_count; i++) {
				dect_ports[i] = &brcm_ports[i];
			}
			led_config[1].state = LS_UNKNOWN;
			led_config[1].name = LN_VOICE2;
			led_config[1].ports = dect_ports;
			led_config[1].num_ports = dect_line_count;

			PORT_MAP** fxs_ports = calloc(fxs_line_count, sizeof(PORT_MAP*));
			for (i = 0; i < fxs_line_count; i++) {
				fxs_ports[i] = &brcm_ports[dect_line_count + i];
			}
			led_config[0].state = LS_UNKNOWN;
			led_config[0].name = LN_VOICE1;
			led_config[0].ports = fxs_ports;
			led_config[0].num_ports = fxs_line_count;
		}
		else {
			/*
			 * LED1 = FXS1, LED2 = FXS2
			 */
			printf("Dual LED configuration, FXS1 and FXS2\n");
			PORT_MAP** fxs1 = calloc(1, sizeof(PORT_MAP*));
			fxs1[0] = &brcm_ports[0];
			led_config[0].state = LS_UNKNOWN;
			led_config[0].name = LN_VOICE1;
			led_config[0].ports = fxs1;
			led_config[0].num_ports = 1;

			PORT_MAP** fxs2 = calloc(1, sizeof(PORT_MAP*));
			fxs2[0] = &brcm_ports[1];
			led_config[1].state = LS_UNKNOWN;
			led_config[1].name = LN_VOICE2;
			led_config[1].ports = fxs2;
			led_config[1].num_ports = 1;
		}
	}

	//Now add all accounts that have incoming calls to one of the governing ports
	for (i = 0; i < voice_led_count; i++) {
		Led* led = &led_config[i];
		led->peers = calloc(MAX_SIP_PEERS, sizeof(SIP_PEER*));
		led->num_peers = 0;

		SIP_PEER *peers = sip_peers;
		while (peers->account.id != SIP_ACCOUNT_UNKNOWN) {
			int is_added = 0;
			/* Skip if SIP account is not enabled */
			if (!uci_get_peer_enabled(peers)) {
				peers++;
				continue;
			}

			const char* call_lines = uci_get_called_lines(peers);
			if (call_lines) {
				char buf[20];
				char *delimiter = " ";
				char *value;
				int line_id = -1;
				strncpy(buf, call_lines, 20);
				value = strtok(buf, delimiter);

				//Check all ports called by this account (numbers 0 to x)
				while (value != NULL) {
					line_id = atoi(value);
					//Check if this port is among the governing ports for this led
					int j;
					for (j = 0; j < led->num_ports; j++) {
						if (led->ports[j]->port == line_id) {

							//printf("LED %d governed by PEER %s\n", led->name, peers->account.name);
							//This is a matching peer
							led->peers[led->num_peers] = peers;
							led->num_peers++;
							is_added = 1;
							break;
						}
					}
					if (is_added) {
						break; //break out here if peer has been added
					}
					else {
						value = strtok(NULL, delimiter);
					}
				}
			}
			peers++;
		}
	}
}

void init_brcm_ports() {
	PORT_MAP *ports;

	ports = brcm_ports;
	while (ports->port != PORT_UNKNOWN) {
		ports->off_hook = 0;
		strcpy(ports->sub[0].state, "ONHOOK");
		strcpy(ports->sub[1].state, "ONHOOK");
		ports++;
	}
}

void init_sip_peers() {
	const SIP_ACCOUNT *accounts;

	accounts = sip_accounts;
	for (;;) {
		sip_peers[accounts->id].account.id = accounts->id;
		strcpy(sip_peers[accounts->id].account.name, accounts->name);
		sip_peers[accounts->id].sip_registry_registered = 0;
		sip_peers[accounts->id].sip_registry_request_sent = 0;
		sip_peers[accounts->id].sip_registry_time = 0;
		sip_peers[accounts->id].ip_list_length = 0;

		/* Init sip show registry data */
		strcpy(sip_peers[accounts->id].username, "Unknown");
		strcpy(sip_peers[accounts->id].domain, "Unknown");
		strcpy(sip_peers[accounts->id].state, "Unknown");
		sip_peers[accounts->id].port = 0;
		sip_peers[accounts->id].domain_port = 0;
		sip_peers[accounts->id].refresh = 0;
		sip_peers[accounts->id].registration_time = 0;

		/* No need to (re)initialize ubus_object (created once at startup) */

		if (accounts->id == SIP_ACCOUNT_UNKNOWN) {
			break;
		}
		accounts++;
	}
}

/*********************************
 * AMI functions and callbacks
 *********************************/
void ami_handle_event(ami_connection* con, ami_event event)
{
	switch (event.type) {
		case LOGIN:
			if (state == CONNECTED) {
				printf("Sending login to AMI, username %s\n", username);
				ami_send_login(con, username, password, on_login_response);
			}
			else {
				printf("Got unexpected LOGIN event\n");
				ami_disconnect(con);
			}
			break;
		case REGISTRY:
			handle_registry_event(event);
			printf("Sending sip show registry\n");
			ami_send_sip_show_registry(con, 0);
			break;
		case REGISTRY_ENTRY:
			handle_registry_entry_event(event);
			break;
		case REGISTRATIONS_COMPLETE:
			printf("Sip show registry complete\n");
			break;
		case BRCM:
			handle_brcm_event(con, event);
			break;
		case CHANNELRELOAD:
			if (event.channel_reload_event->channel_type == CHANNELRELOAD_SIP_EVENT) {
				printf("SIP channel was reloaded\n");
				init_sip_peers(); //SIP has reloaded, initialize sip peer structs
				configure_leds(); //Reconfigure leds, as SIP channel reload may indicate a change to config
			}
			else {
				printf("Unknown channel was reloaded\n");
			}
			break;
		case FULLYBOOTED:
			printf("Asterisk is fully booted\n");
			set_state(LOGGED_IN, con);
			break;
		case VARSET:
			handle_varset_event(event);
			break;
		case DISCONNECT:
			printf("AMI disconnected\n");
			set_state(DISCONNECTED, con);
			break;
		case UNKNOWN_EVENT:
			break; //An event that ami_connection could not parse
		default:
			break; //An event that we dont handle
	}

	manage_leds();
}

void handle_registry_event(ami_event event)
{
	const SIP_ACCOUNT* accounts = sip_accounts;
	SIP_PEER *peer = &sip_peers[PORT_UNKNOWN];
	char* account_name = event.registry_event->account_name;

	//Lookup peer by account name
	while (accounts->id != SIP_ACCOUNT_UNKNOWN) {
		if (!strcmp(accounts->name, account_name)) {
			peer = &sip_peers[accounts->id];
			break;
		}
		accounts++;
	}

	if (peer->account.id == SIP_ACCOUNT_UNKNOWN) {
		printf("Registry event for unknown account: %s\n", account_name);
		return;
	}

	switch (event.registry_event->status) {
		case REGISTRY_REGISTERED_EVENT:
			printf("sip registry registered\n");
			peer->sip_registry_registered = 1;
			peer->sip_registry_request_sent = 0;
			time(&(peer->sip_registry_time)); //Last registration time
			ubus_send_sip_event(peer, "registered", peer->sip_registry_registered);
			ubus_send_sip_event(peer, "registry_request_sent", peer->sip_registry_request_sent);
			handle_iptables(peer, 1);
			break;
		case REGISTRY_UNREGISTERED_EVENT:
			printf("sip registry unregistered\n");
			peer->sip_registry_registered = 0;
			peer->sip_registry_request_sent = 0;
			ubus_send_sip_event(peer, "registered", peer->sip_registry_registered);
			ubus_send_sip_event(peer, "registry_request_sent", peer->sip_registry_request_sent);
			handle_iptables(peer, 0);
			break;
		case REGISTRY_REQUEST_SENT_EVENT:
			if (peer->sip_registry_request_sent == 1) {
				//This means we sent a "REGISTER" without receiving "Registered" event
				peer->sip_registry_registered = 0;
				handle_iptables(peer, 0);
			}
			peer->sip_registry_request_sent = 1;
			ubus_send_sip_event(peer, "registered", peer->sip_registry_registered);
			ubus_send_sip_event(peer, "registry_request_sent", peer->sip_registry_request_sent);
			break;
		default:
			break;
	}
}

void handle_registry_entry_event(ami_event event)
{
	printf("Info: Got registry entry event for SIP account %s\n", event.registry_entry_event->host);
	const SIP_ACCOUNT* accounts = sip_accounts;
	SIP_PEER *peer = &sip_peers[PORT_UNKNOWN];
	char* account_name = event.registry_entry_event->host;

	//Lookup peer by account name
	while (accounts->id != SIP_ACCOUNT_UNKNOWN) {
		if (!strcmp(accounts->name, account_name)) {
			peer = &sip_peers[accounts->id];
			break;
		}
		accounts++;
	}

	if (peer->account.id == SIP_ACCOUNT_UNKNOWN) {
		printf("RegistryEntry event for unknown account: %s\n", account_name);
		return;
	}
	
	//Update our sip peer with event information
	strncpy(peer->username, event.registry_entry_event->username, MAX_SIP_PEER_USERNAME);
	peer->username[MAX_SIP_PEER_USERNAME - 1]= '\0';
	strncpy(peer->domain, event.registry_entry_event->domain, MAX_SIP_PEER_DOMAIN);
	peer->domain[MAX_SIP_PEER_USERNAME - 1]= '\0';
	strncpy(peer->state, event.registry_entry_event->state, MAX_SIP_PEER_STATE);
	peer->state[MAX_SIP_PEER_USERNAME - 1]= '\0';
	peer->port = event.registry_entry_event->port;
	peer->domain_port = event.registry_entry_event->domain_port;
	peer->refresh = event.registry_entry_event->refresh;
	peer->registration_time = event.registry_entry_event->registration_time;
}

void handle_brcm_event(ami_connection* con, ami_event event)
{
	int line_id;
	int subchannel_id;

	switch (event.brcm_event->type) {
		case BRCM_STATUS_EVENT:
			printf("Got BRCM_STATUS_EVENT for %d, offhook = %d\n", event.brcm_event->status.line_id, event.brcm_event->status.off_hook);
			line_id = event.brcm_event->status.line_id;
			if (line_id >= 0 && line_id < PORT_ALL) {
				brcm_ports[line_id].off_hook = event.brcm_event->status.off_hook;
			}
			else {
				printf("Got BRCM Status event for unknown line %d\n", line_id);
			}
			break;
		case BRCM_STATE_EVENT:
			printf("Got BRCM_STATE_EVENT for %d.%d: %s\n", event.brcm_event->state.line_id, event.brcm_event->state.subchannel_id, event.brcm_event->state.state);
			line_id = event.brcm_event->state.line_id;
			subchannel_id = event.brcm_event->state.subchannel_id;

			if (line_id >= 0 && line_id < PORT_ALL) {
				strcpy(brcm_ports[line_id].sub[subchannel_id].state, event.brcm_event->state.state);
				char* subchannel = subchannel_id ? "0" : "1";
				ubus_send_brcm_event(&brcm_ports[line_id], subchannel, brcm_ports[line_id].sub[subchannel_id].state);
			}
			else {
				printf("Got BRCM Status event for unknown line %d\n", line_id);
			}
			break;
		case BRCM_MODULE_EVENT:
			if (event.brcm_event->module_loaded) {
				printf("BRCM module loaded\n");
				ami_send_brcm_ports_show(con, on_brcm_ports_show_response);
			}
			else {
				printf("BRCM module unloaded\n");
				set_state(LOGGED_IN, con);
			}
			break;
		default:
			break;
	}
}

void handle_varset_event(ami_event event)
{
	if (event.varset_event->channel && event.varset_event->variable && event.varset_event->value) {
		/* Event contained all vital parts, send ubus event */
		blob_buf_init(&bb, 0);
		blobmsg_add_string(&bb, event.varset_event->variable, event.varset_event->value);
		ubus_send_event(ctx, event.varset_event->channel, bb.head);
	}
}


/*
 * Callback to handle login result
 */
void on_login_response(ami_connection* con, char* buf)
{
	if (strstr(buf, "Success")) {
		printf("Log in successful\n");
	}
	else {
		printf("Log in failed\n");
		ami_disconnect(con);
	}
}

/*
 * Callback to handle response to brcm module show action.
 */
void on_brcm_module_show_response(ami_connection* con, char* buf)
{
	if (strstr(buf, "1 modules loaded")) {
		printf("BRCM channel driver is loaded\n");
		ami_send_brcm_ports_show(con, on_brcm_ports_show_response);
	}
	else {
		printf("BRCM channel driver is not loaded\n");
	}
}

/*
 * Callback to handle response to brcm ports show
 */
void on_brcm_ports_show_response(ami_connection* con, char* buf)
{
	int result = 1;

	char* fxs_needle = strstr(buf, "FXS");
	if (fxs_needle == NULL) {
		printf("Could not find number of FXS ports\n");
		result = 0;
	}
	else {
		fxs_line_count = strtol(fxs_needle + 4, NULL, 10);
		printf("Found %d FXS ports\n", fxs_line_count);
	}

	char* dect_needle = strstr(buf, "DECT");
	if (dect_needle == NULL) {
		printf("Could not find number of DECT ports\n");
		result = 0;
	}
	else {
		dect_line_count = strtol(dect_needle + 5, NULL, 10);
		printf("Found %d DECT ports\n", dect_line_count);
	}

	if (result) {
		set_state(READY, con);
	}
}

/******************************
 * UBUS functions and structs
 ******************************/

static void system_fd_set_cloexec(int fd)
{
#ifdef FD_CLOEXEC
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
}

enum {
	UBUS_ARG0,
	__UBUS_ARGMAX,
};

/*
 * ubus string argument policy
 */
static const struct blobmsg_policy ubus_string_argument[__UBUS_ARGMAX] = {
	[UBUS_ARG0] = { .name = "info", .type = BLOBMSG_TYPE_STRING },
};

/*
 * Sends asterisk.sip events
 */
static int ubus_send_sip_event(const SIP_PEER *peer, const char *key, const int value)
{
	char id[BUFLEN];
	char sValue[BUFLEN];

	snprintf(id, BUFLEN, "asterisk.sip.%d", peer->account.id);
	snprintf(sValue, BUFLEN, "%d", value);

	blob_buf_init(&bb, 0);
	blobmsg_add_string(&bb, key, sValue);

	return ubus_send_event(ctx, id, bb.head);
}

/*
 * Sends asterisk.brcm events
 */
static int ubus_send_brcm_event(const PORT_MAP *port, const char *key, const char *value)
{
	char id[BUFLEN];
	snprintf(id, BUFLEN, "asterisk.brcm.%d", port->port);

	blob_buf_init(&bb, 0);
	blobmsg_add_string(&bb, key, value);

	return ubus_send_event(ctx, id, bb.head);
}

/*
 * Collects and returns information on a single brcm line to a ubus message buffer
 */
static int ubus_get_brcm_line(struct blob_buf *b, int line)
{
	if (line >= PORT_UNKNOWN || line >= PORT_ALL || line < 0) {
		return 1;
	}

	const PORT_MAP *p = &(brcm_ports[line]);
	blobmsg_add_string(b, "sub_0_state", p->sub[0].state);
	blobmsg_add_string(b, "sub_1_state", p->sub[1].state);

	return 0;
}

/*
 * Collects and returns information on a single sip account to a ubus message buffer
 */
static int ubus_get_sip_account(struct blob_buf *b, int account_id)
{
	if (account_id >= SIP_ACCOUNT_UNKNOWN || account_id < 0) {
		return 1;
	}

	blobmsg_add_u8(b, "registered", sip_peers[account_id].sip_registry_registered);
	blobmsg_add_u8(b, "registry_request_sent", sip_peers[account_id].sip_registry_request_sent);

	//IP address(es) of the sip registrar
	int i;
	for (i = 0; i<sip_peers[account_id].ip_list_length; i++) {
		blobmsg_add_string(b, "ip", sip_peers[account_id].ip_list[i].addr);
	}

	blobmsg_add_u32(b, "port", sip_peers[account_id].port);
	blobmsg_add_string(b, "username", sip_peers[account_id].username);
	blobmsg_add_string(b, "domain", sip_peers[account_id].domain);
	blobmsg_add_u32(b, "domain_port", sip_peers[account_id].domain_port);
	blobmsg_add_u32(b, "refresh_interval", sip_peers[account_id].refresh);
	blobmsg_add_string(b, "state", sip_peers[account_id].state);

	//Format registration time
	if (sip_peers[account_id].registration_time > 0) {
		struct tm* timeinfo;
		char buf[80];
		timeinfo = localtime(&(sip_peers[account_id].registration_time));
		strftime(buf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
		blobmsg_add_string(b, "registration_time", buf);
	}
	else {
		blobmsg_add_string(b, "registration_time", "-");
	}

	//This is the time of last successful registration for this account,
	//regardless of the current registration state (differs from registration_time)
	if (sip_peers[account_id].sip_registry_time > 0) {
		struct tm* timeinfo;
		char buf[80];
		timeinfo = localtime(&(sip_peers[account_id].sip_registry_time));
		strftime(buf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
		blobmsg_add_string(b, "last_successful_registration", buf);
	}
	else {
		blobmsg_add_string(b, "last_successful_registration", "-");
	}

	return 0;
}

/*
 * ubus callback that replies to "asterisk.brcm.X status"
 */
static int ubus_asterisk_brcm_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];

	blobmsg_parse(ubus_string_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));
	blob_buf_init(&bb, 0);

	PORT_MAP *port;
	port = brcm_ports;
	while (port->port != PORT_UNKNOWN) {
		if (port->ubus_object == obj) {
			ubus_get_brcm_line(&bb, port->port); //Add port status to message
			break;
		}
		port++;
	}

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

/*
 * ubus callback that replies to "asterisk.sip.X status"
 */
static int ubus_asterisk_sip_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];

	blobmsg_parse(ubus_string_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));
	blob_buf_init(&bb, 0);

	SIP_PEER *peer = sip_peers;
	while (peer->account.id != SIP_ACCOUNT_UNKNOWN) {
		if (peer->ubus_object == obj) {
			ubus_get_sip_account(&bb, peer->account.id); //Add SIP account status to message
			break;
		}
		peer++;
	}

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

/*
 * ubus callback that replies to "asterisk.codecs status"
 */
static int ubus_codecs_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];
	struct codec *codec;
	struct codec *codec_tmp;

	blobmsg_parse(ubus_string_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));
	blob_buf_init(&bb, 0);

	codec = uci_get_codecs();

	while (codec) {
		/* Node with next == NULL serves as end marker */
		if (codec->next == NULL) {
			codec_delete(codec);
			break;
		}

		void *table = blobmsg_open_table(&bb, codec->key);
		blobmsg_add_string(&bb, "name", codec->value);
		if (codec->bitrate) {
			blobmsg_add_u32(&bb, "bitrate", codec->bitrate);
		}
		blobmsg_close_table(&bb, table);

		codec_tmp = codec;
		codec = codec->next;
		codec_delete(codec_tmp);
	}

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

static struct codec *codec_create()
{
	struct codec *c = malloc(sizeof(struct codec));
	bzero(c, sizeof(struct codec));

	return c;
}

static void codec_delete(struct codec *c)
{
	if (c->key) {
		free(c->key);
	}

	if (c->value) {
		free(c->value);
	}

	free(c);
}

/*
 * uci callback, called for each "supported_codec" found
 */
static void codec_cb(const char * name, void *priv)
{
	struct codec *c = (struct codec *) priv;

	/* Store key/value to last codec in list */
	while (c->next) {
		c = c->next;
	}
	c->key = strdup(name);
	c->value = strdup(ucix_get_option(uci_ctx, UCI_VOICE_PACKAGE, name, "name"));
	const char *bitrate = ucix_get_option(uci_ctx, UCI_VOICE_PACKAGE, name, "bitrate");
	c->bitrate = bitrate ? atoi(bitrate) : 0;

	/* Create space for next codec */
	c->next = codec_create();
}

/*
 * ubus callback that replies to "asterisk status".
 * Recursively reports status for all lines/accounts
 */
static int ubus_asterisk_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];

	blobmsg_parse(ubus_string_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));
	blob_buf_init(&bb, 0);

	SIP_PEER *peer = sip_peers;
	void *sip_table = blobmsg_open_table(&bb, "sip");
	while (peer->account.id != SIP_ACCOUNT_UNKNOWN) {
		void *sip_account_table = blobmsg_open_table(&bb, peer->account.name);
		ubus_get_sip_account(&bb, peer->account.id); //Add SIP account status to message
		blobmsg_close_table(&bb, sip_account_table);
		peer++;
	}
	blobmsg_close_table(&bb, sip_table);

	PORT_MAP *port = brcm_ports;
	void *brcm_table = blobmsg_open_table(&bb, "brcm");
	while (port->port != PORT_UNKNOWN && port->port != PORT_ALL) {
		void *line_table = blobmsg_open_table(&bb, port->name);
		ubus_get_brcm_line(&bb, port->port); //Add port status to message
		blobmsg_close_table(&bb, line_table);
		port++;
	}
	blobmsg_close_table(&bb, brcm_table);

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

static struct ubus_method sip_object_methods[] = {
	{ .name = "status", .handler = ubus_asterisk_sip_cb },
};

static struct ubus_object_type sip_object_type =
	UBUS_OBJECT_TYPE("sip_object", sip_object_methods);

static struct ubus_object ubus_sip_objects[] = {
	{ .name = "asterisk.sip.0", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.1", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.2", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.3", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.4", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.5", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.6", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
	{ .name = "asterisk.sip.7", .type = &sip_object_type, .methods = sip_object_methods, .n_methods = ARRAY_SIZE(sip_object_methods) },
};

static struct ubus_method brcm_object_methods[] = {
	{ .name = "status", .handler = ubus_asterisk_brcm_cb },
};

static struct ubus_object_type brcm_object_type =
	UBUS_OBJECT_TYPE("brcm_object", brcm_object_methods);

static struct ubus_object ubus_brcm_objects[] = {
	{ .name = "asterisk.brcm.0", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
	{ .name = "asterisk.brcm.1", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
	{ .name = "asterisk.brcm.2", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
	{ .name = "asterisk.brcm.3", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
	{ .name = "asterisk.brcm.4", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
	{ .name = "asterisk.brcm.5", .type = &brcm_object_type, .methods = brcm_object_methods, .n_methods = ARRAY_SIZE(brcm_object_methods) },
};

static struct ubus_method asterisk_object_methods[] = {
	{ .name = "status", .handler = ubus_asterisk_cb },
	{ .name = "codecs", .handler = ubus_codecs_cb },
};

static struct ubus_object_type asterisk_object_type =
	UBUS_OBJECT_TYPE("asterisk_object", asterisk_object_methods);

static struct ubus_object ubus_asterisk_object = {
		.name = "asterisk",
		.type = &asterisk_object_type,
		.methods = asterisk_object_methods,
		.n_methods = ARRAY_SIZE(asterisk_object_methods) };

static int ubus_add_objects(struct ubus_context *ctx)
{
	int ret = 0;

	SIP_PEER *peer;
	peer = sip_peers;
	while (peer->account.id != SIP_ACCOUNT_UNKNOWN) {
		peer->ubus_object = &ubus_sip_objects[peer->account.id];
		ret &= ubus_add_object(ctx, peer->ubus_object);
		peer++;
	}

	PORT_MAP *port;
	port = brcm_ports;
	while (port->port != PORT_ALL) {
		port->ubus_object = &ubus_brcm_objects[port->port];
		ret &= ubus_add_object(ctx, port->ubus_object);
		port++;
	}

	ret &= ubus_add_object(ctx, &ubus_asterisk_object);

	return ret;
}

static void ubus_connection_lost_cb(struct ubus_context *ctx)
{
	fprintf(stderr, "UBUS connection lost\n");
	ubus_connected = false;
}

static void sighandler(int signo)
{
	if (signo == SIGINT) {
		running = false;
	}
}

int main(int argc, char **argv)
{
	signal(SIGINT, sighandler);

	state = DISCONNECTED;

	fd_set fset;				/* FD set */
	struct timeval timeout;  /* Timeout for select */

	/* Count voice leds from uci HW context */
	voice_led_count = uci_get_voice_led_count();

	init_sip_peers();
	log_sip_peers();


	/* Initialize ami connection */
	ami_connection* con = ami_init(ami_handle_event);
	if (ami_connect(con, hostname, portno)) {
		set_state(CONNECTED, con);
	}

	/* Initialize ubus connection and register asterisk object */
	ctx = ubus_connect(NULL);
	if (ctx) {
		ctx->connection_lost = ubus_connection_lost_cb;
		system_fd_set_cloexec(ctx->sock.fd);
		int ret = ubus_add_objects(ctx);
		if (ret == 0) {
			ubus_connected = true;
			printf("Connected to UBUS, id: %08x\n", ctx->local_id);
		}
		else {
			ubus_free(ctx);
			ctx = NULL;
		}
	}
	
	running = true;

	/* Main application loop */
	while(running) {
		FD_ZERO(&fset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (!ctx) {
			ubus_connected = false;
		}

		if (ubus_connected) {
			FD_SET(ctx->sock.fd, &fset);
		}

		if (state != DISCONNECTED) {
			FD_SET(con->sd, &fset);
		}

		/* Wait for events from ubus or ami */
		int err = select(FD_SETSIZE, &fset, NULL, NULL, &timeout);
		if(err < 0) {
			if (errno == EINTR) {
				break;
			}
			fprintf(stderr, "Error: %s\n", strerror(errno));
			continue;
		}

		if (ubus_connected) {
			if (FD_ISSET(ctx->sock.fd, &fset)) {
				//printf("Handling UBUS events\n");
				ubus_handle_event(ctx);
			}
		}
		else {
			if (ctx) {
				if (ubus_reconnect(ctx, NULL) == 0) {
					printf("UBUS reconnected\n");
					ubus_connected = true;
					system_fd_set_cloexec(ctx->sock.fd);
				}
			}
			else {
				ctx = ubus_connect(NULL);
				if (ctx) {
					ctx->connection_lost = ubus_connection_lost_cb;
					system_fd_set_cloexec(ctx->sock.fd);
					int ret = ubus_add_objects(ctx);
					if (ret == 0) {
						ubus_connected = true;
						printf("Connected to UBUS, id: %08x\n", ctx->local_id);
					}
					else {
						ubus_free(ctx);
						ctx = NULL;
					}
				}
			}
		}

		if (state == DISCONNECTED) {
			if (ami_connect(con, hostname, portno)) {
				set_state(CONNECTED, con);
			}
		}
		else {
			if (FD_ISSET(con->sd, &fset)) {
				ami_handle_data(con);
			}
		}
	}

	ubus_free(ctx); //Shut down UBUS connection
	printf("UBUS connection closed\n");
	ami_free(con); //Shut down AMI connection
	printf("AMI connection closed\n");
	free_led_config();
	return 0;
}


void set_state(AMI_STATE new_state, ami_connection* con)
{
	if (state != new_state) {
		state = new_state;
		switch(new_state) {
			case DISCONNECTED:
				printf("In state DISCONNECTED\n");
				fxs_line_count = 0;
				dect_line_count = 0;
				break;
			case CONNECTED:
				printf("In state CONNECTED\n");
				/* We wait for LOGIN event here, then attempt to log in */
				break;
			case LOGGED_IN:
				printf("In state LOGGED_IN\n");
				init_brcm_ports();
				fxs_line_count = 0;
				dect_line_count = 0;
				ami_send_brcm_module_show(con, on_brcm_module_show_response);
				ami_send_sip_reload(con, NULL);
				break;
			case READY:
				printf("In state READY\n");
				configure_leds();
				manage_leds();
				break;
			default:
				break;
		}
	}
}

/*
 * Reload uci context, as any changes to config will not be read otherwise
 */
void ucix_reload()
{
	if (uci_ctx) {
		ucix_cleanup(uci_ctx);
	}
	uci_ctx = ucix_init(UCI_VOICE_PACKAGE);
}
