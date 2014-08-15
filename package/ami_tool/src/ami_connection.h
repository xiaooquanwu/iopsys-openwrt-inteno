/*
 * ami_connection.h
 *
 * Manage connections to AMI.
 *
 * Callbacks in client code will be called when events or responses have been received
 * from AMI. There is no event loop built in, client should call ami_handle_data whenever
 * new data is available on the connections filedescriptor.
 */

#define MESSAGE_FRAME "\r\n\r\n"
#define MESSAGE_FRAME_LOGIN "\r\n"
#define MESSAGE_FRAME_LEN 5

#define AMI_BUFLEN 512

typedef struct {
	enum {
		BRCM_UNKNOWN_EVENT,
		BRCM_STATUS_EVENT,
		BRCM_STATE_EVENT,
		BRCM_MODULE_EVENT
	} type;
	struct {
		int line_id;
		int off_hook;
	} status;
	struct {
		int line_id;
		int subchannel_id;
		char* state;
	} state;
	int module_loaded;
} brcm_event;

typedef struct {
	char* account_name;
	enum {
		REGISTRY_UNKNOWN_EVENT,
		REGISTRY_REQUEST_SENT_EVENT,
		REGISTRY_UNREGISTERED_EVENT,
		REGISTRY_REGISTERED_EVENT
	} status;
} registry_event;

typedef struct {
	char* host;				//Account name (e.g. sip0)
	int port;				//5060
	char* username;			//0510409896
	char* domain;			//62.80.209.10
	int domain_port;		//5060
	int refresh;			//Refresh interval, 285
	char* state;			//Registration state Registered
	int registration_time;	//Registration timestamp, 1401282865
} registry_entry_event;

typedef struct {
	enum {
		CHANNELRELOAD_UNKNOWN_EVENT,
		CHANNELRELOAD_SIP_EVENT
	} channel_type;
} channel_reload_event;

typedef struct {
	char* channel;
	char* variable;
	char* value;
} varset_event;

typedef enum ami_event_type {
	LOGIN,
	REGISTRY,
	REGISTRY_ENTRY,
	REGISTRATIONS_COMPLETE,
	BRCM,
	CHANNELRELOAD,
	FULLYBOOTED,
	VARSET,
	DISCONNECT,
	UNKNOWN_EVENT,
} ami_event_type;

typedef struct ami_event {
	ami_event_type type;
	brcm_event* brcm_event;
	registry_event* registry_event;
	registry_entry_event* registry_entry_event;
	channel_reload_event* channel_reload_event;
	varset_event* varset_event;
} ami_event;

typedef struct ami_connection ami_connection;
typedef struct ami_action ami_action;
typedef void (*ami_event_cb) (ami_connection* con, ami_event event);
typedef void (*ami_response_cb) (ami_connection* con, char* buf);
/*
 * Holds data related to an AMI connection
 */
struct ami_connection {
	int connected; //ami_connection is connected to asterisk
	int sd;
	char message_frame[MESSAGE_FRAME_LEN];
	char left_over[AMI_BUFLEN * 2 + 1];
	ami_event_cb event_callback;
	ami_action* current_action;
};

ami_connection* ami_init(ami_event_cb on_event);
int ami_connect(ami_connection* con, const char* hostname, const char* portno);
void ami_disconnect(ami_connection* con);
void ami_free(ami_connection* con);

/*
 * Call when new data is available
 */
void ami_handle_data(ami_connection* con);

/*
 * Actions
 */
struct ami_action {
	char message[AMI_BUFLEN];
	ami_response_cb callback;
	ami_action* next_action;
};

void ami_send_sip_reload(ami_connection* con, ami_response_cb on_response);
void ami_send_login(ami_connection* con, char*username, char* password, ami_response_cb on_response);
void ami_send_brcm_module_show(ami_connection* con, ami_response_cb on_response);
void ami_send_brcm_ports_show(ami_connection* con, ami_response_cb on_response);
void ami_send_sip_show_registry(ami_connection* con, ami_response_cb on_response);