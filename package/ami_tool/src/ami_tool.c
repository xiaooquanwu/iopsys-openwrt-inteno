#include "ami_tool.h"
#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>

#include <libubus.h>

static bool ubus_connected = false;
static struct ubus_context *ctx = NULL;
static struct blob_buf bb;
static struct blob_buf b_led;

static SIP_PEER sip_peers[SIP_ACCOUNT_UNKNOWN + 1];

static int rtpstart_current = 0;
static int rtpend_current = 0;
static IP* ip_list_current = NULL;
static int ip_list_length_current = 0;

unsigned int portno = "5038"; //AMI port number
char hostname[] = "127.0.0.1"; //AMI hostname
char username[128] = "local"; //AMI username
char password[128] = "local"; //AMI password

char *buf[10*BUFLEN];     /* declare global to avoid stack * 10 to avoid overrunning it just in case */

/* Asterisk states */
int sip_registry_request_sent = 0;
int sip_registry_registered   = 0;
int brcm_port_0_off           = 0;
int brcm_port_1_off           = 0;
int brcm_channel_driver_loaded	= 0;
int asterisk_fully_booted     = 0;

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
	for (i=0; i<peer->ip_list_length; i++) {
		IP ip = peer->ip_list[i];
		if (family ==  ip.family && strcmp(addr, ip.addr) == 0) {
			return;
		}
	}
	strcpy(peer->ip_list[peer->ip_list_length].addr, addr);
	peer->ip_list[peer->ip_list_length].family = family;
	peer->ip_list_length++;
}

int sip_peer_enabled(SIP_PEER *peer) {
	char res[BUFLEN];
	char parameter[BUFLEN];
	sprintf(parameter, "%s.%s.enabled",
		UCI_VOICE_PACKAGE,
		peer->account.name);
	if (uci_show(parameter, &res[0], sizeof(res), 0)) {
		return 0;
	}
	int rv = atoi(res);
	return rv;
}

char *trim_whitespace(char *str)
{
	char *end;
	while (isspace(*str)) {
		str++;
	}
	if(*str == 0) {
		return str;
	}
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) {
		end--;
	}
	*(end+1) = 0;
	return str;
}

int uci_show(char *parameter, char *buf, size_t buflen, int mandatory)
{
	FILE *fp;
	char path[BUFLEN];
	char cmd[BUFLEN];

	/* Open the uci command for reading */
	snprintf(cmd, BUFLEN, "%s show %s%s",
		UCI_BIN,
		mandatory ? "" : "-q ",
		parameter);
	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		return 1;
	}

	/* Read one line of output */
	while (fgets(path, sizeof(path)-1, fp) != NULL) {
		strcpy(buf, path);
		break;
	}
	pclose(fp);

	if (strlen(buf) == 0) {
		if (mandatory) {
			printf("Unable to read uci response\n");
		}
		return 1;
	}

	/* Copy result into buffer */
	char parse_buffer[BUFLEN];
	char *delimiter = "=";
	char *value;
	strcpy(parse_buffer, buf);
	value = strtok(parse_buffer, delimiter);
	if (strcmp(value, parameter)) {
		if (mandatory) {
			printf("Unknown uci parameter\n");
		}
		return 1;
	}
	if (value == NULL || (value = strtok(NULL, delimiter)) == NULL) {
		if (mandatory) {
			printf("Unparsable uci response\n");
		}
		return 1; 
	}
	strcpy(buf, value);
	buf = trim_whitespace(buf);
	return 0;
}

/* Get RTP port range, this is a global configuration */
int get_rtp_port_range(int *rtpstart, int *rtpend)
{
	char parameter[BUFLEN];
	char buf[BUFLEN];

	snprintf(parameter, BUFLEN, "%s.SIP.rtpstart", UCI_VOICE_PACKAGE);
	if (uci_show(parameter, buf, BUFLEN, 0)) {
		return 1;
	}
	*rtpstart = atoi(buf);

	snprintf(parameter, BUFLEN, "%s.SIP.rtpend", UCI_VOICE_PACKAGE);
	if (uci_show(parameter, buf, BUFLEN, 0)) {
		return 1;
	}
	*rtpend = atoi(buf);

	return 0;
}	

/* Get proxy, this is a global configuration */
int get_sip_proxy(char *buf, size_t buflen)
{
	char parameter[BUFLEN];

	snprintf(parameter, BUFLEN, "%s.SIP.sip_proxy", UCI_VOICE_PACKAGE);
	if (uci_show(parameter, buf, buflen, 0)) {
		return 1;
	}

	return 0;
}

/* Get hostname or IP using uci */
int get_sip_domain(SIP_PEER *peer, char *buf, size_t buflen)
{
	char parameter[BUFLEN];
	char *sipaccount;
	int enabled;

	sipaccount = peer->account.name;

	for (;;) {
		/* Get enabled sip account */
		snprintf(parameter, BUFLEN, "%s.%s.enabled", UCI_VOICE_PACKAGE, sipaccount);
		if (uci_show(parameter, buf, buflen, 1)) {
			printf("Failed to check enabled\n");
			return 1;
		}
		enabled = atoi(buf);
		if (!enabled) {
			printf("%s not enabled\n", sipaccount);
			return 1;
		}
		break;
	}

	/* Get sip domain */
	snprintf(parameter, BUFLEN, "%s.%s.domain", UCI_VOICE_PACKAGE, sipaccount);
	if (uci_show(parameter, buf, buflen, 1)) {
		printf("Failed to get domain\n");
		return 1;
	}

	return 0;
}

/* Resolv name into ip (A or AAA record), update IP list for peer */
static int resolv(SIP_PEER *peer, char *domain)
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
	const PORT_MAP *ports;
	SIP_PEER *peer;
	IP *ip_list;

	*ip_list_length = 0;
	ip_list = (IP *) malloc(MAX_IP_LIST_LENGTH * sizeof(struct IP));

	ports = brcm_ports;
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
	int rtpstart, rtpend;

	/* Is there a change in IP or RTP port range? */
	ip_list = create_ip_set(family, &ip_list_length);
	if (get_rtp_port_range(&rtpstart, &rtpend)) {
		printf("RTP port range, using default\n");
		rtpstart = RTP_RANGE_START_DEFAULT;
		rtpend = RTP_RANGE_END_DEFAULT;
	}
	if (compare_ip_set(ip_list_current, ip_list_length_current, ip_list, ip_list_length) == 0 &&
	    rtpstart_current == rtpstart &&
	    rtpend_current == rtpend) {
		printf("No changes in IP or RTP port range\n");
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

	snprintf((char *)&buf, BUFLEN, "/etc/init.d/firewall restart");
	printf("%s\n", buf);
	system(buf);
}

/* Resolv domains and add IPs to iptables */
int handle_iptables(SIP_PEER *peer, int doResolv)
{
	char domain[BUFLEN];

	/* Clear old IP list */
	peer->ip_list_length = 0;

	if (doResolv) {
		printf("reg ok. resolving\n");
		/* Get domain to resolv */
		if (get_sip_domain(peer, domain, BUFLEN)) {
			printf("Failed to get sip domain\n");
			return 1;
		}
		resolv(peer, domain);

		/* Get proxy and resolv if configured */
		if (get_sip_proxy(domain, BUFLEN) == 0) {
			resolv(peer, domain);
		}
	} else {
		printf("reg not ok\n");
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

/*
 * message_frame:  String defining message border
 * buffer:	   Buffer to parse
 * framed_message: Pointer to framed message (mallocd)
 * buffer_read:    Bytes read from buffer in previous run
 */
int parse_buffer(char *message_frame, char *buffer, char **framed_message, int *buffer_read)
{
	//Skip bytes already read
	buffer = &buffer[*buffer_read];

	if (strlen(buffer) == 0) {
		return -1;
	}

	//Locate message frame
	char *message_end = strstr(buffer, message_frame);
	if (!message_end) {
		//Could not find message frame, use left over
		//data in next call to parse_buffer
		return -1;
	}

	//Found a message boundry
	int message_length = message_end - buffer;
	*framed_message = calloc(message_length +1, sizeof(char));
	//*framed_message = (char *) malloc(message_length + 1);
	strncpy(*framed_message, buffer, message_length);
	(*framed_message)[message_length] = '\0';
	printf("Framed message:\n[%s]\n\n", *framed_message);

	//Update byte counter
	*buffer_read += message_length + strlen(message_frame);

	//Find out what type of message this is
	int message_type;
	if (!memcmp(*framed_message, "Asterisk Call Manager", 21)) {
		//printf("Login prompt detected\n");
		message_type = LOGIN;
	} else if(!memcmp(*framed_message, "Event", 5)) {
		//printf("Event detected: ");
		message_type = EVENT;
	} else if(!memcmp(*framed_message, "Response", 8)) {
		//printf("Response detected: ");
		message_type = RESPONSE; 
	} else {
		//printf("Unknown event: ");
		message_type = UNKNOWN_STATE;
	}

	return message_type;
}

int send_login(ami_connection* con) {
	char loginstring[256];
	printf("Sending login\n");
	sprintf(loginstring,"Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);
	write(con->sd, loginstring, strlen(loginstring));
	con->message_frame = MESSAGE_FRAME; //Login sent, now there's always <CR><LF><CR><LR> after a message
	con->logged_in = true;
	return 0;
}

int send_sip_reload(ami_connection* con) {
	char reloadstring[256];
	printf("Sending sip reload\n");
	sprintf(reloadstring,"Action: Command\r\nCommand: sip reload\r\n\r\n");
	write(con->sd, reloadstring, strlen(reloadstring));
	con->pending_command = SIPRELOAD;
	return 0;
}

int send_module_show(ami_connection* con) {
	char moduleshowstring[256];
	printf("Sending module show\n");
	sprintf(moduleshowstring, "Action: Command\r\nCommand: module show like chan_brcm\r\n\r\n");
	write(con->sd, moduleshowstring, strlen(moduleshowstring));
	con->pending_command = MODULESHOW;
	return 0;
}

int send_brcm_dialtone_settings(ami_connection* con, const BRCM_PORT port, const char *dialtone_state) {
	char dialtonestring[256];
	printf("Sending brcm dialtone settings for port %d\n", port);
	sprintf(dialtonestring, "Action: BRCMDialtoneSet\r\nLineId: %d\r\nDialtone: %s\r\n\r\n", port, dialtone_state);
	write(con->sd, dialtonestring, strlen(dialtonestring));
	con->pending_command = BRCMDIALTONESET;
	return 0;
}

int get_event_type(char* buf, int* idx) {
	int i = 0;

	if (!memcmp(buf, "Registry", 8)) {
		i +=8;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return REGISTRY;
	} else if (!memcmp(buf, "BRCM", 4)) {
		i +=8;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return BRCM;
	} else if (!memcmp(buf, "ChannelReload", 13)) {
		i +=8;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return CHANNELRELOAD;
	} else if (!memcmp(buf, "FullyBooted", 11)) {
		i +=11;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return FULLYBOOTED;
	} // else if() handle other events

	while(buf[i] || i>BUFLEN) {
		if (buf[i] == '\n') {
			break;
		}
		i++;
	}
	*idx = i;
	return UNKNOWN_EVENT;
}

//peer name is expected to be on format:
//peer-<port>-<other parts of name>
SIP_ACCOUNT_ID get_sip_account_id(char* buf) {

	char account_name[BUFLEN];
	const SIP_ACCOUNT *accounts;

	strncpy(account_name, buf, sizeof(account_name));
	account_name[BUFLEN-1] = '\0';

	//Clear out remainder of buf
	int i;
	for (i=0; i<strlen(account_name); i++) {
		if (isspace(account_name[i])) {
			account_name[i] = '\0';
			break;
		}
	}

	//Lookup port name
	accounts = sip_accounts;
	while (accounts->id != SIP_ACCOUNT_UNKNOWN) {
		if (!strcmp(accounts->name, account_name)) {
			return accounts->id;
		}
		accounts++;
	}

	return SIP_ACCOUNT_UNKNOWN;
}

int handle_registry_event_type(char* buf, int* idx) {
	int i = 0;
	SIP_ACCOUNT_ID sip_account_id;
	SIP_PEER *peer = &sip_peers[PORT_UNKNOWN];

	while (i<BUFLEN) {
		if (!memcmp(&buf[i], "Domain: ", 8)) {
			i+=8;
			sip_account_id = get_sip_account_id(&buf[i]);
			if (sip_account_id == SIP_ACCOUNT_UNKNOWN) {
				printf("Could not find sip account\n");
				return 0;
			}
			peer = &sip_peers[sip_account_id];

		} else if (!memcmp(&buf[i], "Status: ", 8)) {
			if (peer->account.id == SIP_ACCOUNT_UNKNOWN) {
				printf("Can't map status to sip peer\n");
				return 0;
			}
			i+=8;
			if (!memcmp(&buf[i], "Request Sent", 12)) {
				if (peer->sip_registry_request_sent == 1) {
					//This means we sent a "REGISTER" without receiving "Registered" event
					peer->sip_registry_registered = 0;
					handle_iptables(peer, 0);
				}
				peer->sip_registry_request_sent = 1;
				printf("sip registry request sent\n");
			} else if (!memcmp(&buf[i], "Unregistered", 12)) {
				peer->sip_registry_registered = 0;
				peer->sip_registry_request_sent = 0;
				printf("sip registry unregistered\n");
				handle_iptables(peer, 0);
			} else if (!memcmp(&buf[i], "Registered", 10)) {
				peer->sip_registry_registered = 1;
				peer->sip_registry_request_sent = 0;
				time(&(peer->sip_registry_time)); //Last registration time
				printf("sip registry registered\n");
				handle_iptables(peer, 1);
			}
			return 0;
		} else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i<BUFLEN)
				i++;
			i+=2;
		}
	}

	return 0;
}

int handle_brcm_event_type(char* buf, int* idx) {
	int i = 0;

	while (i<BUFLEN) {
		if (!memcmp(&buf[i], "Status: ", 8)) {
			i+=8;
			if (!memcmp(&buf[i], "OFF", 3)) {
				if (buf[i+4] == '0') {
					brcm_ports[PORT_BRCM1].off_hook = 1;
					brcm_port_1_off = 1;
				} else if (buf[i+4] == '1') {
					brcm_ports[PORT_BRCM0].off_hook = 1;
					brcm_port_0_off = 1;
				}
				printf("brcm off hook [0] = %d, [1] = %d\n", brcm_port_0_off, brcm_port_1_off);
			} else if (!memcmp(&buf[i], "ON", 2)) {
				if (buf[i+3] == '0') {
					brcm_ports[PORT_BRCM1].off_hook = 0;
					brcm_port_1_off = 0;
				} else if (buf[i+3] == '1') {
					brcm_ports[PORT_BRCM0].off_hook = 0;
					brcm_port_0_off = 0;
				}
				printf("brcm on hook\n");
			}
			return 0;
		} else if (!memcmp(&buf[i], "State: ", 7)) {
			i+=7;
			char line_id[BUFLEN];
			char subchannel_id[BUFLEN];
			char state[BUFLEN];

			char parse_buffer[BUFLEN];
			char *delimiter = " ";
			char *value;
			strcpy(parse_buffer, buf+i);

			value = strtok(parse_buffer, delimiter);
			if (!value) {
				printf("Unparsable BRCM state\n");
				return 1;
			}
			strcpy(state, value);
			value = strtok(NULL, delimiter);
			if (!value) {
				printf("Unparsable BRCM state\n");
				return 1;
			}
			strcpy(line_id, value);
			value = strtok(NULL, delimiter);
			if (!value) {
				printf("Unparsable BRCM state\n");
				return 1;
			}
			strcpy(subchannel_id, value);

			char line[BUFLEN];
			char subchannel[BUFLEN];
			sprintf(line, "brcm%s", trim_whitespace(line_id));
			sprintf(subchannel, "subchannel_%s", trim_whitespace(subchannel_id));

			printf("Updated channel state\n");
			//Try to set state on port/subchannel
			strcpy(brcm_ports[atoi(line_id)].sub[atoi(subchannel_id)].state, trim_whitespace(state));
			return 0;
		} else if (!memcmp(&buf[i], "Module unload", 13)) {
			i+=13;
			brcm_channel_driver_loaded = 0;

			/* Reset dialtone state to ensure that new dialtone setting will
			 * be applied when channel driver is loaded */
			PORT_MAP *ports;
			ports = brcm_ports;
			while (ports->port != PORT_UNKNOWN) {
				strcpy(ports->dialtone_state, DEFAULT_DIALTONE_STATE);
				strcpy(ports->new_dialtone_state, "");
				strcpy(ports->sub[0].state, "");
				strcpy(ports->sub[1].state, "");
				ports++;
			}
			return 0;
		} else if (!memcmp(&buf[i], "Module load", 11)) {
			i+=11;
			brcm_channel_driver_loaded = 1;
			return 0;
		} else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i<BUFLEN)
				i++;
			i+=2;
		}
	}

	return 0;
}

int handle_channel_reload_sip()
{
	init_sip_peers();
	return 0;
}

int handle_channel_reload(char* buf, int* idx) {
	int i = 0;

	while (i<BUFLEN) {
		if (!memcmp(&buf[i], "ChannelType: ", 13)) {
			i+=13;
			if (!memcmp(&buf[i], "SIP", 3)) {
				handle_channel_reload_sip();
			}
			return 0;
		} else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i<BUFLEN)
				i++;
			i+=2;
		}
	}
	return 0;
}


int handle_event(ami_connection* con, char* buf) {
	int type;
	int idx = 0;

	type = get_event_type(buf, &idx);

	switch(type) {
		case REGISTRY:
			printf("REGISTRY\n");
			handle_registry_event_type(&buf[idx], &idx);
			break;
		case BRCM:
			printf("BRCM\n");
			handle_brcm_event_type(&buf[idx], &idx);
			break;
		case CHANNELRELOAD:
			printf("CHANNELRELOAD\n");
			handle_channel_reload(&buf[idx], &idx);
			send_module_show(con);
			break;
		case FULLYBOOTED:
			printf("FULLYBOOTED\n");
			asterisk_fully_booted = 1;
			break;
		case UNKNOWN_EVENT:
		default:
			return 1;
	}
	
	return 0;
}

int handle_response(ami_connection* con, char* buf) {
	if (con->pending_command == MODULESHOW) {
		if (strstr(buf, "1 modules loaded")) {
			brcm_channel_driver_loaded = 1;
		}
		printf("BRCM channel driver %sloaded\n", brcm_channel_driver_loaded ? "" : "not ");
	}
	else {
		con->pending_command = COMMAND_NONE;
	}
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

void manage_dialtones(ami_connection* con)
{
	if (brcm_channel_driver_loaded == 0 || asterisk_fully_booted == 0) {
		//Skip until asterisk has been started and channel driver has been loaded
		return;
	}

	SIP_PEER *peer;
	const SIP_PEER *peers = sip_peers;
	while (peers->account.id != SIP_ACCOUNT_UNKNOWN) {
		peer = &sip_peers[peers->account.id];

		/* Skip if SIP account is not enabled */
		if (!sip_peer_enabled(peer)) {
			peers++;
			continue;
		}

		/* Get dialtone state to be used for ports under this SIP account */
		char dialtone_state[MAX_DIALTONE_STATE_LENGTH];
		if (peer->sip_registry_registered == 1) {
			strcpy(dialtone_state, "on");
		} else {
			strcpy(dialtone_state, "congestion");
		}

		/* Get ports used for incomming calls to this SIP account */
		char res[BUFLEN];
		char parameter[BUFLEN];
		sprintf(parameter, "%s.%s.call_lines",
			UCI_VOICE_PACKAGE,
			peer->account.name);
		if (uci_show(parameter, &res[0], sizeof(res), 0)) {
			printf("Failed to check call_lines for %s\n", peer->account.name);
		} else {
			char parse_buffer[BUFLEN];
			char *delimiter = " ";
			char *value;
			strcpy(parse_buffer, res);

			value = strtok(parse_buffer, delimiter);
			while (value != NULL) {
				int line_id = atoi(value);
				if (line_id < PORT_ALL) {
					/* Set congestion if not already set (worst case) or on if not already changed */
					if ((!strcmp(dialtone_state, "congestion") && strcmp(brcm_ports[line_id].new_dialtone_state, "congestion")) ||
						(!brcm_ports[line_id].dialtone_dirty && strcmp(brcm_ports[line_id].new_dialtone_state, dialtone_state))) {

						brcm_ports[line_id].dialtone_dirty = 1; //This port needs an update
						strcpy(brcm_ports[line_id].new_dialtone_state, dialtone_state);
					}
					brcm_ports[line_id].dialtone_configured = 1; //This port is configured, don't set to off below
				} else {
					printf("Unparsable line id in %s.call_lines\n", peer->account.name);
				}
				value = strtok(NULL, delimiter);
			}
		}
		peers++;
	}

	PORT_MAP *ports = brcm_ports;
	BRCM_PORT port;
	while (ports->port < PORT_ALL) {
		if (!ports->dialtone_configured && strcmp(ports->dialtone_state, "off")) {
			/* No SIP account is configured to send incomming calls on this line */
			strcpy(ports->new_dialtone_state, "off");
			ports->dialtone_dirty = 1;
		}

		if (ports->dialtone_dirty && strcmp(ports->new_dialtone_state, ports->dialtone_state) && strlen(ports->new_dialtone_state)) {
			/* Dialtone changed, apply in chan_brcm */
			port = ports->port;
			printf("Apply dialtone %s for port %d\n", ports->new_dialtone_state, port);
			strcpy(ports->dialtone_state, ports->new_dialtone_state);
			strcpy(ports->new_dialtone_state, "");
			send_brcm_dialtone_settings(con, port, ports->dialtone_state);
		}

		/* Reset states */	
		ports->dialtone_dirty = 0;
		ports->dialtone_configured = 0;

		ports++;
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

//use something like LED_STATE_MAP to list available states
/*
static void manage_led_ubus(const char *led_name, const char *state)
{
	//Lookup the id of led object
	uint32_t id;
	if (ubus_lookup_id(ctx, led_name, &id)) {
		fprintf(stderr, "Failed to look up %s object\n", led_name);
		return;
	}

	//Specify the state we want to set
	blob_buf_init(&b_led, 0);
	blobmsg_add_string(&b_led, "state", state);

	fprintf(stderr, "Setting LED %s state to %s\n", led_name, state);

	//Set state
	ubus_invoke(ctx, id, "set", b_led.head, NULL, 0, 1000);
}
*/

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
 * BU rewriting manage_leds to use ubus/ledmngr
 *
 * TODO:
 * New led states/new business logic
 * Retrieve brcm ports using AMI request: "Action: BRCMPortsShow"
 * There is a problem after reload where the led will not stop blinking even though account is registered
 * Query the /lib/db/board file for number of voice leds
 */
void manage_leds() {

	if (brcm_channel_driver_loaded == 0 || asterisk_fully_booted == 0) {
		//Skip until asterisk has been started and channel driver has been loaded
		return;
	}

	/*
	 * Voice LED rules:
	 * No accounts configured => led off
	 * Line active/ringing => blinking
	 * Account registered => led on
	 * Account configured but register failed => blinkling
	 */
	const PORT_MAP *ports = brcm_ports;
	LED_STATE state = LS_OFF;
	while (ports->port != PORT_ALL) {
		if (brcm_subchannel_active(ports)) {
			state = LS_NOTICE;
			break;
		}
		ports++;
	}

	if (state == LS_OFF) {
		SIP_PEER *peer;
		const SIP_PEER *peers = sip_peers;
		while (peers->account.id != SIP_ACCOUNT_UNKNOWN) {
			peer = &sip_peers[peers->account.id];

			/* Skip if SIP account is not enabled */
			if (!sip_peer_enabled(peer)) {
				peers++;
				continue;
			}

			if (!peer->sip_registry_registered) {
				state = LS_ERROR;
				break;
			} else {
				if (state != LS_NOTICE && state != LS_ERROR) {
					state = LS_OK;
				}
			}
			peers++;
		}
	}

	manage_led(LN_VOICE1, state);
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
		if (accounts->id == SIP_ACCOUNT_UNKNOWN) {
			break;
		}
		accounts++;
	}
}

/*
 * UBUS functions and structs
 */

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
 * ubus integer argument policy
 */
static const struct blobmsg_policy ubus_int_argument[__UBUS_ARGMAX] = {
	[UBUS_ARG0] = { .name = "id", .type = BLOBMSG_TYPE_INT32 },
};

/*
 * Collects and returns information on a single brcm line to a ubus message buffer
 */
static int ubus_get_brcm_line(struct blob_buf *b, int line)
{
	if (line >= PORT_UNKNOWN || line >= PORT_ALL || line < 0) {
		return 1;
	}

	const PORT_MAP *p = &(brcm_ports[line]);
	blobmsg_add_string(b, "dialtone", p->dialtone_state);
	blobmsg_add_string(b, "sub_0_state", p->sub[0].state);
	blobmsg_add_string(b, "sub_1_state", p->sub[1].state);

	return 0;
}

/*
 * Collects and returns information on all brcm lines to a ubus message buffer
 */
static void ubus_get_brcm_lines(struct blob_buf *b, bool table)
{
	void *brcm_table = NULL;
	void *brcm_line_table = NULL;
	if (table) brcm_table = blobmsg_open_table(b, "brcm");

	int i;
	for (i=0; i<PORT_ALL; i++) {
		brcm_line_table = blobmsg_open_table(b, (brcm_ports[i]).name);
		ubus_get_brcm_line(b, i);
		blobmsg_close_table(b, brcm_line_table);
	}

	if (table) blobmsg_close_table(b, brcm_table);
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

	//format last successful registration time
	if (sip_peers[account_id].sip_registry_time > 0) {
		struct tm* timeinfo;
		char buf[80];
		timeinfo = localtime(&(sip_peers[account_id].sip_registry_time));
		strftime(buf, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
		blobmsg_add_string(b, "last_registration", buf);
	}
	else {
		blobmsg_add_string(b, "last_registration", "-");
	}

	//IP address(es) of the sip registrar
	int i;
	for (i = 0; i<sip_peers[account_id].ip_list_length; i++) {
		blobmsg_add_string(b, "ip", sip_peers[account_id].ip_list[i].addr);
	}

	return 0;
}

/*
 * Collects and adds info on all sip accounts to a ubus message buffer
 */
static void ubus_get_sip_accounts(struct blob_buf *b, bool table)
{
	void *sip_table = NULL;
	void *sip_account_table = NULL;
	if (table) sip_table = blobmsg_open_table(b, "sip");

	const SIP_ACCOUNT *accounts;
	accounts = sip_accounts;
	for (;;) {
		if (accounts->id == SIP_ACCOUNT_UNKNOWN) {
			break;
		}

		sip_account_table = blobmsg_open_table(b, accounts->name);
		ubus_get_sip_account(b, accounts->id);
		blobmsg_close_table(b, sip_account_table);

		accounts++;
	}
	if (table) blobmsg_close_table(b, sip_table);
}


/*
 * ubus callback that sends sip account information.
 * Optional id - integer argument - to specify which account should be returned.
 * Returns all accounts if no arg is given
 */
static int ubus_sip_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];
	blobmsg_parse(ubus_int_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));


	blob_buf_init(&bb, 0);

	if (!tb[UBUS_ARG0]) {
		ubus_get_sip_accounts(&bb, 0); //No account specified, print all sip accounts
	}
	else {
		int sip_account = blobmsg_get_u32(tb[UBUS_ARG0]);
		int result = ubus_get_sip_account(&bb, sip_account);
		if (result > 0) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
	}

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

/*
 * ubus callback that sends brcm line information.
 * Optional id - integer argument - to specify which line should be returned.
 * Returns all lines if no arg is given
 */
static int ubus_brcm_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];
	blobmsg_parse(ubus_int_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));

	blob_buf_init(&bb, 0);

	if (!tb[UBUS_ARG0]) {
		ubus_get_brcm_lines(&bb, 0); //No argument, print all brcm lines
	}
	else {
		int brcm_line = blobmsg_get_u32(tb[UBUS_ARG0]);
		int result = ubus_get_brcm_line(&bb, brcm_line);

		if (result > 0) {
			return UBUS_STATUS_INVALID_ARGUMENT;
		}
	}

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

/*
 * ubus callback that sends all sip and brcm information to ubus.
 */
static int ubus_asterisk_cb (
	struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method,
	struct blob_attr *msg)
{
	struct blob_attr *tb[__UBUS_ARGMAX];

	blobmsg_parse(ubus_string_argument, __UBUS_ARGMAX, tb, blob_data(msg), blob_len(msg));
	blob_buf_init(&bb, 0);

	ubus_get_sip_accounts(&bb, 1); //Add SIP info to message
	ubus_get_brcm_lines(&bb, 1); //Add brcm info to message

	ubus_send_reply(ctx, req, bb.head);
	return 0;
}

static struct ubus_method asterisk_object_methods[] = {
	{ .name = "info", .handler = ubus_asterisk_cb },
	UBUS_METHOD("sip", ubus_sip_cb, ubus_int_argument),
	UBUS_METHOD("brcm", ubus_brcm_cb, ubus_int_argument),
};

static struct ubus_object_type asterisk_object_type =
	UBUS_OBJECT_TYPE("system", asterisk_object_methods);

static struct ubus_object router_object = {
	.name = "asterisk",
	.type = &asterisk_object_type,
	.methods = asterisk_object_methods,
	.n_methods = ARRAY_SIZE(asterisk_object_methods),
};

static void ubus_connection_lost_cb(struct ubus_context *ctx)
{
	fprintf(stderr, "UBUS connection lost\n");
	ubus_connected = false;
}

/* AMI Connection management */

static ami_connection* ami_init()
{
	ami_connection* con;
	con = calloc(1, sizeof(*con));

	con->connected = false;
	con->logged_in = false;
	con->sd = -1;
	con->message_frame = NULL;
	con->state = 0;
	con->pending_command = COMMAND_NONE;
	memset(con->left_over, 0, BUFLEN * 2 + 1);
	return con;
}

static void ami_disconnect(ami_connection* con)
{
	close(con->sd);
	con->sd = -1;
	con->connected = false;
	con->logged_in = false;
}

static void ami_free(ami_connection* con) {
	ami_disconnect(con);
	free(con);
}

static void ami_connect(ami_connection* con)
{
	ami_disconnect(con);
	con->message_frame = MESSAGE_FRAME_LOGIN;
	asterisk_fully_booted = 0;
	brcm_channel_driver_loaded = 0;

	struct addrinfo *host;
	int err = getaddrinfo(hostname, portno, NULL, &host);
	if (err) {
		fprintf(stderr, "Unable to connect to AMI: %s\n", gai_strerror(err));
		con->connected = false;
		return;
	}
	con->sd = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(con->sd, host->ai_addr, host->ai_addrlen);
	if (res == 0) {
		printf("Connected to AMI\n");
		con->connected = true;
	}
	else {
		fprintf(stderr, "Unable to connect to AMI: %s\n", strerror(errno));
		con->connected = false;
	}
	freeaddrinfo(host);
}

static void ami_handle_data(ami_connection* con)
{
	int idx = 0; //buffer position
	char* message = NULL;
	char buf[BUFLEN * 2 + 1];
	int handled = 0;

	//Read data from ami
	memset(buf, 0, BUFLEN * 2 + 1);
	if (read(con->sd, buf, BUFLEN-1) <= 0) {
		con->connected = false;
		return; //we have been disconnected
	}

	//Concatenate left over data with newly read
	if (strlen(con->left_over)) {
		char tmp[BUFLEN * 2 + 1];
		strcpy(tmp, con->left_over);
		strcat(tmp, buf);
		strcpy(buf, tmp);
		con->left_over[0] = '\0';
	}

	while ((con->state = parse_buffer(con->message_frame, buf, &message, &idx)) != -1) {
		switch (con->state) {
			case LOGIN:
				send_login(con);
				sleep(0.1); //give server time to reply
				send_sip_reload(con);
				handled = 1;
				break;
			case EVENT:
				if (!handle_event(con, message+7)) { //string is "Event: ...."
					handled = 1;
				}
				break;
			case RESPONSE:
				handle_response(con, message);
				handled = 1;
				break;
			default:
				break;
		}
		free(message);
	}

	//Unable to frame data in buffer, store remaining buffer until next packet is read
	if (con->state == -1 && idx < strlen(buf)) {
		strcpy(con->left_over, &buf[idx]);
	}

	if (handled) {
		manage_dialtones(con);
		manage_leds();
	}
}

int main(int argc, char **argv)
{
	fd_set fset;				/* FD set */
	struct timeval timeout;  /* Timeout for select */

	init_sip_peers();
	log_sip_peers();

	/* Initialize ami connection */
	ami_connection* con = ami_init();
	ami_connect(con);
	if (!con->connected) {
		return -1;
	}

	/* Initialize ubus connection and register asterisk object */
	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to UBUS\n");
		return -1;
	}
	ubus_connected = true;
	printf("Connected to UBUS, id: %08x\n", ctx->local_id);
	ctx->connection_lost = ubus_connection_lost_cb;
	system_fd_set_cloexec(ctx->sock.fd);
	int ret = ubus_add_object(ctx, &router_object);
	if (ret != 0) {
		fprintf(stderr, "Failed to publish object '%s': %s\n", router_object.name, ubus_strerror(ret));
		return -1;
	}

	/* Main application loop */
	while(1) {
		FD_ZERO(&fset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (ubus_connected) {
			FD_SET(ctx->sock.fd, &fset);
		}

		if (con->connected) {
			FD_SET(con->sd, &fset);
		}

		/* Wait for events from ubus or ami */
		int err = select(FD_SETSIZE, &fset, NULL, NULL, &timeout);
		if(err < 0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			if (errno == EINTR) {
				break;
			}
			continue;
		}

		if (ubus_connected) {
			if (FD_ISSET(ctx->sock.fd, &fset)) {
				printf("Handling UBUS events\n");
				ubus_handle_event(ctx);
			}
		}
		else {
			if (ubus_reconnect(ctx, NULL) == 0) {
				system_fd_set_cloexec(ctx->sock.fd);
				ubus_connected = true;
				printf("UBUS reconnected\n");
			}
		}

		if (con->connected) {
			if (FD_ISSET(con->sd, &fset)) {
				printf("Handling AMI events\n");
				ami_handle_data(con);
			}
		}
		else {
			printf("Reconnecting to AMI\n");
			ami_connect(con);
		}
	}

	ubus_free(ctx); //Shut down UBUS connection
	printf("UBUS connection closed\n");
	ami_free(con); //Shut down AMI connection
	printf("AMI connection closed\n");
	return 0;
}
