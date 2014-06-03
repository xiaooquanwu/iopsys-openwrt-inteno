#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>

//For dynamic firewall support
#define IPTABLES_CHAIN "zone_wan_input"
#define IPTABLES_BIN "iptables"
#define IPTABLES_FILE "/etc/firewall.sip"
#ifdef USE_IPV6
#define IP6TABLES_BIN "ip6tables"
#define IP6TABLES_FILE "/etc/firewall6.sip"
#endif
#define ECHO_BIN "echo"
#define UCI_BIN "uci"
#define UCI_VOICE_PACKAGE "voice_client"

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 65
#endif
#define	MAX_IP_LIST_LENGTH	20
#define RTP_RANGE_START_DEFAULT	10000
#define RTP_RANGE_END_DEFAULT	20000

#define BUFLEN 512

typedef struct IP
{
	int	family;
	char	addr[NI_MAXHOST];
} IP;

typedef enum AMI_STATE {
	DISCONNECTED,	//Not connected to AMI
	CONNECTED,		//Connected to AMI
	LOGGED_IN,		//Logged in to AMI
	READY			//Ready to handle all events
} AMI_STATE;
AMI_STATE state;

typedef enum LED_STATE
{
	LS_OK,
	LS_NOTICE,
	LS_ALERT,
	LS_ERROR,
	LS_OFF,
	LS_UNKNOWN
} LED_STATE;

#define MAX_LED_STATE	10
typedef struct LED_STATE_MAP
{
	LED_STATE	state;
	char		str[MAX_LED_STATE];
} LED_STATE_MAP;

static const LED_STATE_MAP led_states[] =
{
	{LS_OK,		"ok"},
	{LS_NOTICE,	"notice"},
	{LS_ALERT,	"alert"},
	{LS_ERROR,	"error"},
	{LS_OFF,	"off"},
	{LS_UNKNOWN,	"-"},
};

typedef enum LED_NAME
{
	LN_DSL,
	LN_WIFI,
	LN_WPS,
	LN_LAN,
	LN_STATUS,
	LN_DECT,
	LN_TV,
	LN_USB,
	LN_WAN,
	LN_INTERNET,
	LN_VOICE1,
	LN_VOICE2,
	LN_ECO,
	LN_ALL,
	LN_UNKNOWN
} LED_NAME;

#define MAX_LED_NAME	13
typedef struct LED_NAME_MAP
{
	LED_NAME	name;
	char		str[MAX_LED_NAME];
} LED_NAME_MAP;

static const LED_NAME_MAP led_names[]  =
{
	{LN_DSL,		"led.dsl"},
	{LN_WIFI,		"led.wifi"},
	{LN_WPS,		"led.wps"},
	{LN_LAN,		"led.lan"},
	{LN_STATUS,		"led.status"},
	{LN_DECT,		"led.dect"},
	{LN_TV,			"led.tv"},
	{LN_USB,		"led.usb"},
	{LN_WAN,		"led.wan"},
	{LN_INTERNET,	"led.internet"},
	{LN_VOICE1,		"led.voice1"},
	{LN_VOICE2,		"led.voice2"},
	{LN_ECO,		"led.eco"},
	{LN_ALL,		"All"},
	{LN_UNKNOWN,	"-"}
};

typedef struct LED_CURRENT_STATE_MAP
{
	LED_NAME	name;
	LED_STATE	state;
} LED_CURRENT_STATE_MAP;

static LED_CURRENT_STATE_MAP led_current_states[]  =
{
	{LN_VOICE1,		LS_UNKNOWN},
	{LN_VOICE2,		LS_UNKNOWN},
	{LN_UNKNOWN,	LS_UNKNOWN}
};

//These are used to map SIP peer name to a port
//CPE may be configured to share the same SIP-account for several ports or to use individual accounts
typedef enum BRCM_PORT
{
	PORT_BRCM0 = 0,
	PORT_BRCM1,
	PORT_BRCM2,
	PORT_BRCM3,
	PORT_BRCM4,
	PORT_BRCM5,
	PORT_ALL,
	PORT_UNKNOWN
} BRCM_PORT;

typedef struct SUBCHANNEL
{
	char		state[80];
} SUBCHANNEL;

#define MAX_PORT_NAME	10
typedef struct PORT_MAP
{
	char		name[MAX_PORT_NAME];
	BRCM_PORT	port;
	int		off_hook;
	SUBCHANNEL	sub[2]; //TODO define for number of subchannels?
	struct ubus_object *ubus_object;
} PORT_MAP;

static PORT_MAP brcm_ports[] =
{
	{"brcm0",	PORT_BRCM0,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"brcm1",	PORT_BRCM1,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"brcm2",	PORT_BRCM2,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"brcm3",	PORT_BRCM3,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"brcm4",	PORT_BRCM4,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"brcm5",	PORT_BRCM5,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	//Add other ports here as needed
	{"port_all",	PORT_ALL,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
	{"-",		PORT_UNKNOWN,	0,	{ {"ONHOOK"}, {"ONHOOK"} }, NULL },
};

typedef enum SIP_ACCOUNT_ID
{
	SIP_ACCOUNT_0 = 0,
	SIP_ACCOUNT_1,
	SIP_ACCOUNT_2,
	SIP_ACCOUNT_3,
	SIP_ACCOUNT_4,
	SIP_ACCOUNT_5,
	SIP_ACCOUNT_6,
	SIP_ACCOUNT_7,
	SIP_ACCOUNT_UNKNOWN
} SIP_ACCOUNT_ID;

#define MAX_ACCOUNT_NAME	10
typedef struct SIP_ACCOUNT
{
	SIP_ACCOUNT_ID	id;
	char		name[MAX_ACCOUNT_NAME];
} SIP_ACCOUNT;

static const SIP_ACCOUNT sip_accounts[] = {
	{SIP_ACCOUNT_0,		"sip0"},
	{SIP_ACCOUNT_1,		"sip1"},
	{SIP_ACCOUNT_2,		"sip2"},
	{SIP_ACCOUNT_3,		"sip3"},
	{SIP_ACCOUNT_4,		"sip4"},
	{SIP_ACCOUNT_5,		"sip5"},
	{SIP_ACCOUNT_6,		"sip6"},
	{SIP_ACCOUNT_7,		"sip7"},
	{SIP_ACCOUNT_UNKNOWN,	"-"}
};

#define MAX_SIP_PEERS 10
#define MAX_SIP_PEER_NAME 10
#define MAX_SIP_PEER_USERNAME 128
#define MAX_SIP_PEER_DOMAIN 128
#define MAX_SIP_PEER_STATE 128
typedef struct SIP_PEER
{
	SIP_ACCOUNT	account;
	int		sip_registry_request_sent;		//Bool indicating if we have sent a registration request
	int		sip_registry_registered;		//Bool indicating if we are registered or not
	time_t	sip_registry_time;				//The time when we received the registry event
	IP		ip_list[MAX_IP_LIST_LENGTH];	//IP addresses of the sip registrar
	int		ip_list_length;					//Number of addresses

	//Info from sip show registry
	int port;								//The port we are connected to
	char username[MAX_SIP_PEER_USERNAME];	//Our username
	char domain[MAX_SIP_PEER_DOMAIN];		//The domain we are registered on
	int domain_port;						//The domain port
	int refresh;							//Refresh interval for this registration
	char state[MAX_SIP_PEER_STATE];			//Registration state e.g. Registered
	time_t registration_time;				//Registration timestamp, 1401282865

	struct ubus_object *ubus_object;
} SIP_PEER;

/*
 * Struct that stores configuration for a LED
 */
typedef struct {
	LED_STATE state;
	LED_NAME name;
	int num_ports;
	PORT_MAP** ports; //Array of pointers to brcm ports that govern this leds state
	int num_peers;
	SIP_PEER** peers;//Array of pointers to sip peers that govern this leds state
} Led;


void init_sip_peers();
void manage_leds();
void manage_led(LED_NAME led, LED_STATE state);
LED_STATE get_led_state(Led* led);
void configure_leds();
void free_led_config();

