#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ami_connection.h"

typedef enum ami_message {
	UNKNOWN_MESSAGE,
	LOGIN_MESSAGE,
	EVENT_MESSAGE,
	RESPONSE_MESSAGE
} ami_message;

/*
 * message_frame:  String defining message border
 * buffer:	   Buffer to parse
 * framed_message: Pointer to framed message (mallocd)
 * buffer_read:    Bytes read from buffer in previous run
 */
static ami_message parse_buffer(char *message_frame, char *buffer, char **framed_message, int *buffer_read)
{
	//Skip bytes already read
	buffer = &buffer[*buffer_read];

	if (strlen(buffer) == 0) {
		return UNKNOWN_MESSAGE;
	}

	//Locate message frame
	char *message_end = strstr(buffer, message_frame);
	if (!message_end) {
		//Could not find message frame, use left over
		//data in next call to parse_buffer
		return UNKNOWN_MESSAGE;
	}

	//Found a message boundry
	int message_length = message_end - buffer;

	//printf("Framed message:\n[%s]\n\n", *framed_message);

	//Update byte counter
	*buffer_read += message_length + strlen(message_frame);

	//Find out what type of message this is
	ami_message message_type;

	if (!strncmp(buffer, "Asterisk Call Manager", 21)) {
		//printf("Login prompt detected\n");
		message_type = LOGIN_MESSAGE;
	} else if(!strncmp(buffer, "Event", 5)) {
		//printf("Event detected: ");
		message_type = EVENT_MESSAGE;
	} else if(!strncmp(buffer, "Response", 8)) {
		//printf("Response detected: ");
		message_type = RESPONSE_MESSAGE;
	} else {
		//printf("Unknown event: ");
		return UNKNOWN_MESSAGE;
	}

	*framed_message = malloc(message_length +1);
	strncpy(*framed_message, buffer, message_length);
	(*framed_message)[message_length] = '\0';

	return message_type;
}

/*
 * Find the type on an event and advance the idx buffer pointer
 * to the beginning of the event.
 */
static ami_event_type get_event_type(char* buf, int* idx) {
	int i = 0;

	if (!memcmp(buf, "RegistryEntry", 13)) {
		i +=13;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return REGISTRY_ENTRY;
	} else if (!memcmp(buf, "Registry", 8)) {
		i +=8;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return REGISTRY;
	} else if (!memcmp(buf, "RegistrationsComplete", 4)) {
		i +=21;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return REGISTRATIONS_COMPLETE;
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
	} else if (!memcmp(buf, "VarSet", 6)) {
		i +=6;
		while((buf[i] == '\n') || (buf[i] == '\r'))
			i++;

		*idx = i;
		return VARSET;

	} // else if() handle other events

	while(buf[i] || i > AMI_BUFLEN) {
		if (buf[i] == '\n') {
			break;
		}
		i++;
	}
	*idx = i;
	printf("Unhandled event\n%s\n", buf);
	return UNKNOWN_EVENT;
}

static char *trim_whitespace(char *str)
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

ami_event parse_registry_event(char* buf)
{
	ami_event event;
	event.type = REGISTRY;
	event.registry_event = malloc(sizeof(registry_event));
	event.registry_event->status = REGISTRY_UNKNOWN_EVENT;
	event.registry_event->account_name = NULL;

	char* domain = strstr(buf, "Domain: ");
	if (domain) {
		domain += 8; //Increment pointer to start of domain name
		int len = 0;
		while (domain[len] && !isspace(domain[len])) {
			len++;
		}
		char* account_name = calloc(len + 1, sizeof(char));
		strncpy(account_name, domain, len);
		event.registry_event->account_name = account_name;
		printf("Found domain: %s of length %d\n", account_name, len);
	}
	else {
		printf("Warning: No domain found in Registry event\n");
	}

	char* status = NULL;
	if ((status = strstr(buf, "Status: Request Sent"))) {
		event.registry_event->status = REGISTRY_REQUEST_SENT_EVENT;
	}
	else if ((status = strstr(buf, "Status: Unregistered"))) {
		event.registry_event->status = REGISTRY_UNREGISTERED_EVENT;
	}
	else if ((status = strstr(buf, "Status: Registered"))) {
		event.registry_event->status = REGISTRY_REGISTERED_EVENT;
	}
	else {
		printf("Warning: No status found in Registry event\n");
	}

	return event;
}

ami_event parse_registry_entry_event(char* buf)
{
	ami_event event;
	memset(&event, 0, sizeof(event));
	
	event.type = REGISTRY_ENTRY;
	event.registry_entry_event = (registry_entry_event*)malloc(sizeof(registry_entry_event));
	event.registry_entry_event->host = 0;
	event.registry_entry_event->port = 0;
	event.registry_entry_event->username = 0;
	event.registry_entry_event->domain = 0;
	event.registry_entry_event->refresh = 0;
	event.registry_entry_event->state = 0;
	event.registry_entry_event->registration_time = 0;
	
	char* host_start = strstr(buf, "Host:");
	if (host_start) {
		//Found host
		host_start += 6; //Advance to start of content
		char* host_end = strstr(host_start, "\r\n");
		if (host_end) {
			//Found end of host
			int host_len = host_end - host_start;
			event.registry_entry_event->host = (char*)calloc(host_len + 1, 1);
			strncpy(event.registry_entry_event->host, host_start, host_len);
		}
	}

	char* port_start = strstr(buf, "Port:");
	if (port_start) {
		//Found port
		port_start += 6; //Advance to start of content
		event.registry_entry_event->port = strtol(port_start, NULL, 10);
	}

	char* username_start = strstr(buf, "Username:");
	if (username_start) {
		//Found username
		username_start += 10; //Advance to start of content
		char* username_end = strstr(username_start, "\r\n");
		if (username_end) {
			//Found end of username
			int username_len = username_end - username_start;
			event.registry_entry_event->username = (char*)calloc(username_len + 1, 1);
			strncpy(event.registry_entry_event->username, username_start, username_len);
		}
	}	

	char* domain_start = strstr(buf, "Domain:");
	if (domain_start) {
		//Found domain
		domain_start += 8; //Advance to start of content
		char* domain_end = strstr(domain_start, "\r\n");
		if (domain_end) {
			//Found end of domain
			int domain_len = domain_end - domain_start;
			event.registry_entry_event->domain = (char*)calloc(domain_len + 1, 1);
			strncpy(event.registry_entry_event->domain, domain_start, domain_len);
		}
	}

	char* domain_port_start = strstr(buf, "DomainPort:");
	if (domain_port_start) {
		//Found port
		domain_port_start += 12; //Advance to start of content
		event.registry_entry_event->port = strtol(domain_port_start, NULL, 10);
	}

	char* refresh_start = strstr(buf, "Refresh:");
	if (refresh_start) {
		//Found refresh interval
		refresh_start += 9; //Advance to start of content
		event.registry_entry_event->refresh = strtol(refresh_start, NULL, 10);
	}

	char* state_start = strstr(buf, "State:");
	if (state_start) {
		//Found state
		state_start += 7; //Advance to start of content
		char* state_end = strstr(state_start, "\r\n");
		if (state_end) {
			//Found end of state
			int state_len = state_end - state_start;
			event.registry_entry_event->state = (char*)calloc(state_len + 1, 1);
			strncpy(event.registry_entry_event->state, state_start, state_len);
		}
	}

	char* registration_time_start = strstr(buf, "RegistrationTime:");
	if (registration_time_start) {
		//Found reg timestamp
		registration_time_start += 18; //Advance to start of content
		event.registry_entry_event->registration_time = strtol(registration_time_start, NULL, 10);
	}

	return event;
}

ami_event parse_brcm_event(char* buf)
{
	ami_event event;
	memset(&event, 0, sizeof(event));

	event.type = BRCM;
	event.brcm_event = malloc(sizeof(brcm_event));
	event.brcm_event->type = BRCM_UNKNOWN_EVENT;

	char* event_type = NULL;
	char parse_buffer[AMI_BUFLEN];
	char delimiter = ' ';
	char *value;

	if ((event_type = strstr(buf, "Status: "))) {
		event.brcm_event->type = BRCM_STATUS_EVENT;
		strncpy(parse_buffer, event_type + 8, AMI_BUFLEN);
		parse_buffer[AMI_BUFLEN -1] = '\0';
		//strcpy(parse_buffer, event_type + 8);

		value = strtok(parse_buffer, &delimiter);
		if (value && !strcmp(value, "OFF")) {
			event.brcm_event->status.off_hook = 1;
		}
		else if (value && !strcmp(value, "ON")) {
			event.brcm_event->status.off_hook = 0;
		}
		else {
			printf("Warning: No/Unknown status in brcm status event\n");
		}

		value = strtok(NULL, &delimiter);
		if (value) {
			event.brcm_event->status.line_id = strtol(value, NULL, 10);
		}
		else {
			printf("Warning: No/Unknown line id in brcm status event\n");
			event.brcm_event->status.line_id = 0;
		}
	}
	else if ((event_type = strstr(buf, "State: "))) {
		event.brcm_event->type = BRCM_STATE_EVENT;
		strncpy(parse_buffer, event_type + 7, AMI_BUFLEN);
		parse_buffer[AMI_BUFLEN -1] = '\0';
		//strcpy(parse_buffer, event_type + 7);

		value = strtok(parse_buffer, &delimiter);
		if (value) {
			value = trim_whitespace(value);
			event.brcm_event->state.state = calloc(strlen(value) + 1, sizeof(char));
			strcpy(event.brcm_event->state.state, value);
		}
		else {
			printf("Warning: No state in brcm state event\n");
			event.brcm_event->state.state = NULL;
		}

		value = strtok(NULL, &delimiter);
		if (value) {
			event.brcm_event->state.line_id = strtol(value, NULL, 10);
		}
		else {
			printf("Warning: No line_id in brcm state event\n");
			event.brcm_event->state.line_id = -1;
		}

		value = strtok(NULL, &delimiter);
		if (value) {
			event.brcm_event->state.subchannel_id = strtol(value, NULL, 10);
		}
		else {
			printf("Warning: No subchannel_id in brcm state event\n");
			event.brcm_event->state.subchannel_id = -1;
		}
	}
	else if ((event_type = strstr(buf, "Module unload"))) {
		event.brcm_event->type = BRCM_MODULE_EVENT;
		event.brcm_event->module_loaded = 0;
	}
	else if ((event_type = strstr(buf, "Module load"))) {
		event.brcm_event->type = BRCM_MODULE_EVENT;
		event.brcm_event->module_loaded = 1;
	}

	return event;
}

ami_event parse_varset_event(char* buf)
{
	int len;
	ami_event event;
	memset(&event, 0, sizeof(event));
	event.type = VARSET;
	event.varset_event = malloc(sizeof(varset_event));

	char* channel = strstr(buf, "Channel: ");
	if (channel) {
		channel += 9; //Increment pointer to start of channel
		len = 0;
		while (channel[len] && !isspace(channel[len])) {
			len++;
		}
		event.varset_event->channel = calloc(len + 1, sizeof(char));
		strncpy(event.varset_event->channel, channel, len);
	}
	else {
		printf("Warning: No Channel in varset event\n");
		event.varset_event->channel = NULL;
	}

	char* variable = strstr(buf, "Variable: ");
	if (variable) {
		variable += 10; //Increment pointer to start of variable
		len = 0;
		while (variable[len] && !isspace(variable[len])) {
			len++;
		}
		event.varset_event->variable = calloc(len + 1, sizeof(char));
		strncpy(event.varset_event->variable, variable, len);
	}
	else {
		printf("Warning: No Variable in varset event\n");
		event.varset_event->variable = NULL;
	}

	char* value = strstr(buf, "Value: ");
	if (value) {
		value += 7; //Increment pointer to start of value
		len = 0;
		while (value[len] && !isspace(value[len])) {
			len++;
		}
		event.varset_event->value = calloc(len + 1, sizeof(char));
		strncpy(event.varset_event->value, value, len);
	}
	else {
		printf("Warning: No Value in varset event\n");
		event.varset_event->value = NULL;
	}
	return event;
}

ami_event parse_channel_reload_event(char* buf) {
	ami_event event;
	memset(&event, 0, sizeof(event));
	event.type = CHANNELRELOAD;
	event.channel_reload_event = malloc(sizeof(channel_reload_event));

	char* result;
	if ((result = strstr(buf, "ChannelType: SIP"))) {
		event.channel_reload_event->channel_type = CHANNELRELOAD_SIP_EVENT;
	}
	else {
		printf("Warning: unknown channel in ChannelReload event\n");
		event.channel_reload_event->channel_type = CHANNELRELOAD_UNKNOWN_EVENT;
	}
	return event;
}

ami_event parse_fully_booted_event(char* buf) {
	ami_event event;
	memset(&event, 0, sizeof(event));
	event.type = FULLYBOOTED;
	return event;
}

void ami_free_event(ami_event event) {
	switch (event.type) {
		case REGISTRY:
			free(event.registry_event->account_name);
			free(event.registry_event);
			break;
		case BRCM:
			if (event.brcm_event->type == BRCM_STATE_EVENT) {
				free(event.brcm_event->state.state);
			}
			free(event.brcm_event);
			break;
		case CHANNELRELOAD:
			free(event.channel_reload_event);
			break;
		case VARSET:
			free(event.varset_event->channel);
			free(event.varset_event->value);
			free(event.varset_event->variable);
			free(event.varset_event);
			break;
		case REGISTRY_ENTRY:
			free(event.registry_entry_event->host);
			free(event.registry_entry_event->domain);
			free(event.registry_entry_event->state);
			free(event.registry_entry_event->username);
			free(event.registry_entry_event);
			break;
		case FULLYBOOTED:
		case LOGIN:
		case DISCONNECT:
		case REGISTRATIONS_COMPLETE:
			/* no event data to free */
			break;
		case UNKNOWN_EVENT:
			break;
		default:
			printf("WARNING: Unhandled event type, can't free (may leak)\n");
			break;
	}
}

static void ami_handle_event(ami_connection* con, char* message)
{
	int idx = 0;
	ami_event_type type = get_event_type(message, &idx);
	ami_event event;
	memset(&event, 0, sizeof(event));

	switch(type) {
		case BRCM:
			event = parse_brcm_event(&message[idx]);
			break;
		case CHANNELRELOAD:
			event = parse_channel_reload_event(&message[idx]);
			break;
		case FULLYBOOTED:
			event = parse_fully_booted_event(&message[idx]);
			break;
		case VARSET:
			event = parse_varset_event(&message[idx]);
			break;
		case REGISTRY:
			event = parse_registry_event(&message[idx]);
			break;
		case REGISTRY_ENTRY:
			event = parse_registry_entry_event(&message[idx]);
			break;
		case REGISTRATIONS_COMPLETE:
			/*
			 * Probably not needed.
			 * (this happens after all registry entry events have been received)
			 * Event: RegistrationsComplete
			 * EventList: Complete
			 * ListItems: 1
			*/
			event.type = REGISTRATIONS_COMPLETE;
			break;
		case UNKNOWN_EVENT:
		default:
			event.type = UNKNOWN_EVENT;
			break;
	}

	//Let client handle the event
	if (con->event_callback) {
		con->event_callback(con, event);
	}

	ami_free_event(event);
}

static void ami_send_action(ami_connection* con, ami_action* action) {
	if (con->current_action) {
		printf("ERROR: Attempt to send AMI action, but there is already an action pending\n");
		return;
	}
	con->current_action = action;
	write(con->sd, action->message, strlen(action->message));
}

static void ami_handle_response(ami_connection* con, char* message)
{
	ami_action* current = con->current_action;
	assert(current);

	ami_action* next = current->next_action;
	con->current_action = NULL;

	if (next) {
		ami_send_action(con, next);
	}

	if (current->callback) {
		current->callback(con, message);
	}

	free(current);
}

static void queue_action(ami_connection* con, ami_action* action)
{
	action->next_action = NULL;
	if (con->current_action) {
		ami_action* a = con->current_action;
		while(a->next_action) {
			a = a->next_action;
		}
		a->next_action = action;
	}
	else {
		ami_send_action(con, action);
	}
}

/*
 * PUBLIC FUNCTION IMPLEMENTATIONS
 */

ami_connection* ami_init(ami_event_cb on_event) {
	ami_connection* con;
	con = calloc(1, sizeof(*con));

	con->connected = 0;
	con->sd = -1;
	bzero(con->message_frame, sizeof(con->message_frame));
	memset(con->left_over, 0, AMI_BUFLEN * 2 + 1);
	con->current_action = NULL;
	con->event_callback = on_event;

	return con;
}

int ami_connect(ami_connection* con, const char* hostname, const char* portno)
{
	ami_disconnect(con);
	int result = 0;
	strncpy(con->message_frame, MESSAGE_FRAME_LOGIN, MESSAGE_FRAME_LEN);

	struct addrinfo *host;
	int err = getaddrinfo(hostname, portno, NULL, &host);
	if (err) {
		fprintf(stderr, "Unable to connect to AMI: %s\n", gai_strerror(err));
		con->connected = 0;
		return result;
	}
	con->sd = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(con->sd, host->ai_addr, host->ai_addrlen);
	if (res == 0) {
		//printf("Connected to AMI\n");
		con->connected = 1;
		result = 1;
	}
	else {
		fprintf(stderr, "Unable to connect to AMI: %s\n", strerror(errno));
		con->connected = 0;
		result = 0;
	}
	freeaddrinfo(host);
	return result;
}

void ami_disconnect(ami_connection* con)
{
	if (con->connected) {
		close(con->sd);
		con->sd = -1;
		con->connected = 0;

		//Let client know about disconnect
		ami_event event;
		memset(&event, 0, sizeof(event));
		event.type = DISCONNECT;
		con->event_callback(con, event);
	}
}

void ami_free(ami_connection* con) {
	ami_disconnect(con);
	free(con);
}

/*
 * Called by client when ami_connection has new data to process
 */
void ami_handle_data(ami_connection* con)
{
	//printf("Handling data on AMI connection\n");
	int idx = 0; //buffer position
	char* message = NULL;
	char buf[AMI_BUFLEN * 2 + 1];

	//Read data from ami
	memset(buf, 0, AMI_BUFLEN * 2 + 1);
	if (read(con->sd, buf, AMI_BUFLEN-1) <= 0) {
		ami_disconnect(con); //we have been disconnected
		return;
	}

	//Concatenate left over data with newly read
	if (strlen(con->left_over)) {
		char tmp[AMI_BUFLEN * 2 + 1];
		strcpy(tmp, con->left_over);
		strcat(tmp, buf);
		strcpy(buf, tmp);
		con->left_over[0] = '\0';
	}

	ami_message message_type = UNKNOWN_MESSAGE;
	ami_event event;
	memset(&event, 0, sizeof(event));

	while(idx < strlen(buf)) {
		message_type = parse_buffer(con->message_frame, buf, &message, &idx);
		if (message_type == UNKNOWN_MESSAGE) {
			break;
		}
		switch (message_type) {
			case LOGIN_MESSAGE:
				//Send login event to client (time to log in...)
				event.type = LOGIN;
				con->event_callback(con, event);
				ami_free_event(event);
				break;
			case EVENT_MESSAGE:
				ami_handle_event(con, message + 7);
				break;
			case RESPONSE_MESSAGE:
				ami_handle_response(con, message);
				break;
			default:
				printf("Unknown data from AMI: %s\n", message);
				break;
		}
		free(message);
	}

	//store remaining buffer until next packet is read
	if (idx < strlen(buf)) {
		strcpy(con->left_over, &buf[idx]);
	}
}

/*
 * ACTIONS
 * Send an Action to AMI. We expect a response to this, so its possible to provide
 * a callback that will be executed when a response is retrieved. There can only
 * be one action pending at a time. If one is already pending, any new attempts
 * will be ignored.
 */

/*
 * Send command to reload sip channel.
 * CHANNELRELOAD event will be received when reload is completed.
 *
 * Example response:
 * "Response: Follows
 * Privilege: Command
 * --END COMMAND--"
 */
void ami_send_sip_reload(ami_connection* con, ami_response_cb on_response) {
	//printf("Queueing Action: ami_send_sip_reload\n");
	ami_action* action = malloc(sizeof(ami_action));
	action->callback = on_response;
	sprintf(action->message,"Action: Command\r\nCommand: sip reload\r\n\r\n");
	queue_action(con, action);
}

/*
 * Send username and password to AMI
 *
 * Example response:
 * "Response: Success
 * Message: Authentication accepted"
 */
void ami_send_login(ami_connection* con, char* username, char* password, ami_response_cb on_response)
{
	//printf("Queueing Action: ami_send_login\n");
	strncpy(con->message_frame, MESSAGE_FRAME, MESSAGE_FRAME_LEN); //Login sent, now there's always <CR><LF><CR><LR> after a message
	ami_action* action = malloc(sizeof(ami_action));
	action->callback = on_response;
	sprintf(action->message,"Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);
	queue_action(con, action);
}

/*
 * Request an indication on if BRCM module is loaded or not
 *
 * Example response:
 * "Response: Follows
 * Privilege: Command
 * --END COMMAND--"
 */
void ami_send_brcm_module_show(ami_connection* con, ami_response_cb on_response) {
	//printf("Queueing Action: ami_send_brcm_module_show\n");
	ami_action* action = malloc(sizeof(ami_action));
	action->callback = on_response;
	sprintf(action->message, "Action: Command\r\nCommand: module show like chan_brcm\r\n\r\n");
	queue_action(con, action);
}

/*
 * Request an indication on the port configuration
 *
 * Example response:
 * "Response: Success
 * Message:
 * FXS 2
 * DECT 4"
 */
void ami_send_brcm_ports_show(ami_connection* con, ami_response_cb on_response) {
	//printf("Queueing Action: ami_send_brcm_ports_show\n");
	ami_action* action = malloc(sizeof(ami_action));
	action->callback = on_response;
	sprintf(action->message, "Action: BRCMPortsShow\r\n\r\n");
	queue_action(con, action);
}

/*
 * Request SIP Registry information
 * (response is a simple message, then the registry events
 * follow separately, one per registered account)
 * Example response:
 * Response: Success
 * EventList: start
 * Message: Registrations will follow
 * 
 * Event: RegistryEntry
 * Host: sip0
 * Port: 5060
 * Username: 0510409896
 * Domain: 62.80.209.10
 * DomainPort: 5060
 * Refresh: 5385
 * State: Registered
 * RegistrationTime: 1401282865
 * 
 * Event: RegistrationsComplete
 * EventList: Complete
 * ListItems: 1
 */
 void ami_send_sip_show_registry(ami_connection* con, ami_response_cb on_response) {
 	ami_action* action = (ami_action*)malloc(sizeof(ami_action));
 	action->callback = on_response;
 	sprintf(action->message, "Action: SIPshowregistry\r\n\r\n");
 	queue_action(con, action);
 }
