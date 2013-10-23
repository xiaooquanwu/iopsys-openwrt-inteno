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
	*framed_message = calloc(message_length +1, sizeof(char));
	//*framed_message = (char *) malloc(message_length + 1);
	strncpy(*framed_message, buffer, message_length);
	(*framed_message)[message_length] = '\0';
	//printf("Framed message:\n[%s]\n\n", *framed_message);

	//Update byte counter
	*buffer_read += message_length + strlen(message_frame);

	//Find out what type of message this is
	ami_message message_type;
	if (!memcmp(*framed_message, "Asterisk Call Manager", 21)) {
		//printf("Login prompt detected\n");
		message_type = LOGIN_MESSAGE;
	} else if(!memcmp(*framed_message, "Event", 5)) {
		//printf("Event detected: ");
		message_type = EVENT_MESSAGE;
	} else if(!memcmp(*framed_message, "Response", 8)) {
		//printf("Response detected: ");
		message_type = RESPONSE_MESSAGE;
	} else {
		//printf("Unknown event: ");
		message_type = UNKNOWN_MESSAGE;
	}

	return message_type;
}

/*
 * Find the type on an event and advance the idx buffer pointer
 * to the beginning of the event.
 */
static ami_event_type get_event_type(char* buf, int* idx) {
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
	int i = 0;
	ami_event event;
	event.type = REGISTRY;
	event.registry_event = malloc(sizeof(registry_event));
	event.registry_event->status = REGISTRY_UNKNOWN_EVENT;
	event.registry_event->account_name = NULL;

	while (i < AMI_BUFLEN) {
		if (!memcmp(&buf[i], "Domain: ", 8)) {
			i+=8;

			int j = i;
			while (!isspace(buf[i]) && i < AMI_BUFLEN) { //Find first space, thats the end of name
				i++;
			}

			char* account_name = calloc(1+i-j, sizeof(char));
			strncpy(account_name, &buf[j], i-j);
			event.registry_event->account_name = account_name;
		}
		else if (!memcmp(&buf[i], "Status: ", 8)) {
			i+=8;

			if (!memcmp(&buf[i], "Request Sent", 12)) {
				event.registry_event->status = REGISTRY_REQUEST_SENT_EVENT;
			} else if (!memcmp(&buf[i], "Unregistered", 12)) {
				event.registry_event->status = REGISTRY_UNREGISTERED_EVENT;
			} else if (!memcmp(&buf[i], "Registered", 10)) {
				event.registry_event->status = REGISTRY_REGISTERED_EVENT;
			}
		}
		else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			i+=2;
		}
	}

	return event;
}

ami_event parse_brcm_event(char* buf)
{
	ami_event event;
	event.type = BRCM;
	event.brcm_event = malloc(sizeof(brcm_event));
	event.brcm_event->type = BRCM_UNKNOWN_EVENT;

	int i = 0;

	while (i < AMI_BUFLEN) {
		if (!memcmp(&buf[i], "Status: ", 8)) {
			i+=8;
			event.brcm_event->type = BRCM_STATUS_EVENT;

			if (!memcmp(&buf[i], "OFF", 3)) {
				event.brcm_event->status.off_hook = 1;
				i += 4;
			}
			else if (!memcmp(&buf[i], "ON", 2)) {
				event.brcm_event->status.off_hook = 0;
				i += 3;
			}
			int line_id = strtol(buf + i, NULL, 10);
			event.brcm_event->status.line_id = line_id;
			return event;
		}
		else if (!memcmp(&buf[i], "State: ", 7)) {
			i+=7;
			event.brcm_event->type = BRCM_STATE_EVENT;

			char parse_buffer[AMI_BUFLEN];
			char *delimiter = " ";
			char *value;
			strcpy(parse_buffer, buf + i);

			value = strtok(parse_buffer, delimiter);
			if (value) {
				value = trim_whitespace(value);
				event.brcm_event->state.state = calloc(strlen(value) + 1, sizeof(char));
				strcpy(event.brcm_event->state.state, value);
			}
			else {
				event.brcm_event->state.state = NULL;
			}

			value = strtok(NULL, delimiter);
			if (value) {
				event.brcm_event->state.line_id = strtol(value, NULL, 10);
			}
			else {
				event.brcm_event->state.line_id = -1;
			}

			value = strtok(NULL, delimiter);
			if (value) {
				event.brcm_event->state.subchannel_id = strtol(value, NULL, 10);
			}
			else {
				event.brcm_event->state.subchannel_id = -1;
			}

			return event;
		}
		else if (!memcmp(&buf[i], "Module unload", 13)) {
			event.brcm_event->type = BRCM_MODULE_EVENT;
			event.brcm_event->module_loaded = 0;
			return event;
		}
		else if (!memcmp(&buf[i], "Module load", 11)) {
			event.brcm_event->type = BRCM_MODULE_EVENT;
			event.brcm_event->module_loaded = 1;
			return event;
		}
		else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			i+=2;
		}
	}
	return event;
}

ami_event parse_varset_event(char* buf)
{
	int i = 0;
	ami_event event;
	event.type = VARSET;
	event.varset_event = malloc(sizeof(varset_event));
	event.varset_event->channel = NULL;
	event.varset_event->value = NULL;
	event.varset_event->variable = NULL;

	while (i < AMI_BUFLEN) {
		if (!memcmp(&buf[i], "Channel: ", 9)) {
			i+=9;
			int j = i;
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			event.varset_event->channel = calloc(1+i-j, sizeof(char));
			strncpy(event.varset_event->channel, buf + j, i-j);
		}
		else if (!memcmp(&buf[i], "Variable: ", 10)) {
			i+=10;
			int j = i;
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			event.varset_event->variable = calloc(1+i-j, sizeof(char));
			strncpy(event.varset_event->variable, buf + j, i-j);
		}
		else if (!memcmp(&buf[i], "Value: ", 7)) {
			i+=7;
			int j = i;
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			event.varset_event->value = calloc(1+i-j, sizeof(char));
			strncpy(event.varset_event->value, buf + j, i-j);
		} else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			i+=2;
		}
	}
	return event;
}

ami_event parse_channel_reload_event(char* buf) {
	int i = 0;
	ami_event event;
	event.type = CHANNELRELOAD;
	event.channel_reload_event = malloc(sizeof(channel_reload_event));
	event.channel_reload_event->channel_type = CHANNELRELOAD_UNKNOWN_EVENT;

	while (i < AMI_BUFLEN) {
		if (!memcmp(&buf[i], "ChannelType: ", 13)) {
			i+=13;
			if (!memcmp(&buf[i], "SIP", 3)) {
				event.channel_reload_event->channel_type = CHANNELRELOAD_SIP_EVENT;
			}
			break;
		} else {
			//find end of line \r\n
			while(memcmp(&buf[i], "\r\n",2) && i < AMI_BUFLEN)
				i++;
			i+=2;
		}
	}
	return event;
}

ami_event parse_fully_booted_event(char* buf) {
	ami_event event;
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
		case FULLYBOOTED:
		case LOGIN:
		case DISCONNECT:
		case UNKNOWN_EVENT:
		default:
			/* no event data to free */
			break;
	}
}

static void ami_handle_event(ami_connection* con, char* message)
{
	int idx = 0;
	ami_event_type type = get_event_type(message, &idx);
	ami_event event;

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

static void ami_handle_response(ami_connection* con, char* message)
{
	 if (con->response_callback) {
		 ami_response_cb cb = con->response_callback;
		 con->response_callback = NULL;
		 cb(con, message);
	}
}

/*
 * Action handling
 */
static int ami_send_action(ami_connection* con, char* data, ami_response_cb on_response) {
	if (con->response_callback) {
		printf("Can not send Action! There is already an Action pending\n");
		return 1; //Cant have two actions waiting for response at the same time
	}
	write(con->sd, data, strlen(data));
	con->response_callback = on_response;
	return 0;
}

/*
 * PUBLIC FUNCTION IMPLEMENTATIONS
 */

ami_connection* ami_init(ami_event_cb on_event) {
	ami_connection* con;
	con = calloc(1, sizeof(*con));

	con->connected = 0;
	con->sd = -1;
	con->message_frame = NULL;
	memset(con->left_over, 0, AMI_BUFLEN * 2 + 1);

	con->response_callback = NULL;
	con->event_callback = on_event;

	return con;
}

void ami_connect(ami_connection* con, const char* hostname, const char* portno)
{
	ami_disconnect(con);

	con->message_frame = MESSAGE_FRAME_LOGIN;

	struct addrinfo *host;
	int err = getaddrinfo(hostname, portno, NULL, &host);
	if (err) {
		fprintf(stderr, "Unable to connect to AMI: %s\n", gai_strerror(err));
		con->connected = 0;
		return;
	}
	con->sd = socket(AF_INET, SOCK_STREAM, 0);
	int res = connect(con->sd, host->ai_addr, host->ai_addrlen);
	if (res == 0) {
		printf("Connected to AMI\n");
		con->connected = 1;
	}
	else {
		fprintf(stderr, "Unable to connect to AMI: %s\n", strerror(errno));
		con->connected = 0;
	}
	freeaddrinfo(host);
}

void ami_disconnect(ami_connection* con)
{
	close(con->sd);
	con->sd = -1;
	con->connected = 0;

	//Let client know about disconnect
	ami_event event;
	event.type = DISCONNECT;
	con->event_callback(con, event);
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
int ami_send_sip_reload(ami_connection* con, ami_response_cb on_response) {
	char reloadstring[256];
	sprintf(reloadstring,"Action: Command\r\nCommand: sip reload\r\n\r\n");
	return ami_send_action(con, reloadstring, on_response);
}

/*
 * Send username and password to AMI
 *
 * Example response:
 * "Response: Success
 * Message: Authentication accepted"
 */
int ami_send_login(ami_connection* con, char* username, char* password, ami_response_cb on_response)
{
	char loginstring[256];
	sprintf(loginstring,"Action: Login\r\nUsername: %s\r\nSecret: %s\r\n\r\n", username, password);
	con->message_frame = MESSAGE_FRAME; //Login sent, now there's always <CR><LF><CR><LR> after a message
	return ami_send_action(con, loginstring, on_response);
}

/*
 * Request an indication on if BRCM module is loaded or not
 *
 * Example response:
 * "Response: Follows
 * Privilege: Command
 * --END COMMAND--"
 */
int ami_send_brcm_module_show(ami_connection* con, ami_response_cb on_response) {
	char moduleshowstring[256];
	sprintf(moduleshowstring, "Action: Command\r\nCommand: module show like chan_brcm\r\n\r\n");
	return ami_send_action(con, moduleshowstring, on_response);
}

/*
 * Set dialtone for a specific line
 */
int ami_send_brcm_dialtone_settings(ami_connection* con, const int line_id, const char *dialtone_state, ami_response_cb on_response) {
	char dialtonestring[256];
	sprintf(dialtonestring, "Action: BRCMDialtoneSet\r\nLineId: %d\r\nDialtone: %s\r\n\r\n", line_id, dialtone_state);
	return ami_send_action(con, dialtonestring, on_response);
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
int ami_send_brcm_ports_show(ami_connection* con, ami_response_cb on_response) {
	char portshowstring[256];
	sprintf(portshowstring, "Action: BRCMPortsShow\r\n\r\n");
	return ami_send_action(con, portshowstring, on_response);
}
