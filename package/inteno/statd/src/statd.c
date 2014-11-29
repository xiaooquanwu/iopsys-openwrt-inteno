#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <syslog.h>

#include <libubox/blobmsg.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include <uci.h>

#include "statd_rules.h"

#define CFG_PATH "/etc/config/"
#define CFG_FILE "statd"

static struct ubus_event_handler ubus_listener_syslog;

static bool ubus_connected = false;
static struct ubus_context *ubus_ctx = NULL;

static void system_fd_set_cloexec(int fd)
{
#ifdef FD_CLOEXEC
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
#endif
}

static void ubus_receive_event_syslog_cb(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
	char *tmp;
	char *str;

	tmp = blobmsg_format_json(msg, true);
	str = (char *) malloc(strlen(type) + strlen(tmp) + /* Curly braces, null terminator etc... */ 9);
	sprintf(str, "{ \"%s\": %s }", type, tmp);
	printf("Sending to syslog: %s\n", str);
	syslog(LOG_INFO, str);

	free(str);
	free(tmp);
}

static void ubus_connection_lost_cb(struct ubus_context *ctx)
{
	fprintf(stderr, "UBUS connection lost\n");
	ubus_connected = false;
}

static struct ubus_event_handler *get_ubus_event_handler(const struct statd_rule *rule)
{
	static struct ubus_event_handler *ubus_listener = NULL;
	switch (statd_rule_get_destination(rule)) {
		case DEST_SYSLOG:
			ubus_listener = &ubus_listener_syslog;
			break;
		default:
			fprintf(stderr, "Unknown destination, can't register\n");
			break;
	}

	return ubus_listener;
}

static int load_rules()
{
	struct uci_context *uci_ctx = uci_alloc_context();
	if (!uci_ctx) {
		fprintf(stderr, "Failed to initialize uci\n");
		return 1;
	}
	uci_set_confdir(uci_ctx, CFG_PATH);
	if (uci_load(uci_ctx, CFG_FILE, NULL) != UCI_OK) {
		fprintf(stderr, "Configuration missing or corrupt (%s/%s)\n", CFG_PATH, CFG_FILE);
		uci_free_context(uci_ctx);
		return 1;
	}

	struct uci_element *package_element;
	uci_foreach_element(&uci_ctx->root, package_element) {
		struct uci_package *package = uci_to_package(package_element);
	
		struct uci_element *section_element;
		uci_foreach_element(&package->sections, section_element)
		{
			struct uci_section *section = uci_to_section(section_element);
			if (strcmp(section->type, "rule")) {
				fprintf(stderr, "Ignoring unknown uci section type %s\n", section->type);
				continue;
			}

			struct uci_element *option_element;
			const char *filter = NULL;
			enum statd_destination destination = DEST_UNKNOWN;
			uci_foreach_element(&section->options, option_element)
			{
				struct uci_option *option = uci_to_option(option_element);
				if (option->type != UCI_TYPE_STRING) {
					fprintf(stderr, "Ignoring uci option, type is not string\n");
					continue;
				}

				if (!strcmp(option_element->name, "filter")) {
					filter = option->v.string;
				} else if (!strcmp(option_element->name, "destination")) {
					if (strcmp(option->v.string, "syslog")) {
						fprintf(stderr, "Ignoring unknown uci option destination %s\n", option->v.string);
						continue;
					}
					destination = DEST_SYSLOG;
				} else {
					fprintf(stderr, "Ignoring unknown uci option %s\n", option_element->name);
					continue;
				}
			}
			if (filter && destination != DEST_UNKNOWN) {
				statd_rule_add(filter, destination);
			}
		}
	}

	uci_free_context(uci_ctx);

	if (!statd_rule_get_head()) {
		fprintf(stderr, "No valid rules found in configuration\n");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	fd_set fdset;
	struct timeval timeout;

	/* Read configuration */
	if (load_rules()) {
		return 1;
	}

	/* Listener for events to be sent on syslog */
	memset(&ubus_listener_syslog, 0, sizeof(ubus_listener_syslog));
	ubus_listener_syslog.cb = ubus_receive_event_syslog_cb;
	openlog(NULL, LOG_NDELAY, LOG_LOCAL0);

	/* TODO: Listener for events to be sent using snmp trap */
	
	/* Initialize ubus connection */
	ubus_ctx = ubus_connect(NULL);
	if (!ubus_ctx) {
		fprintf(stderr, "Failed to connect to UBUS\n");
	} else {
		ubus_ctx->connection_lost = ubus_connection_lost_cb;
		system_fd_set_cloexec(ubus_ctx->sock.fd);
		printf("Connected to UBUS, id: %08x\n", ubus_ctx->local_id);

		/* Register ubus event listeners */
		ret = 0;
		struct statd_rule *current_rule = statd_rule_get_head();
		while (current_rule) {
			printf("Registering for event: %s\n", statd_rule_get_filter(current_rule));
			struct ubus_event_handler *ubus_listener = get_ubus_event_handler(current_rule);
			if (ubus_listener) {
				ret |= ubus_register_event_handler(ubus_ctx, ubus_listener, statd_rule_get_filter(current_rule));
			}
			current_rule = statd_rule_get_next(current_rule);
		}

		if (ret == 0) {
			ubus_connected = true;
		} else {
			fprintf(stderr, "Error while registering for events: %s\n", ubus_strerror(ret));
			ubus_free(ubus_ctx);
			ubus_ctx = NULL;
		}
	}

	/* Main application loop */
	while(1) {
		FD_ZERO(&fdset);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		if (ubus_connected) {
			FD_SET(ubus_ctx->sock.fd, &fdset);
		}

		/* Wait for events from ubus (or in the future SNMP requests) */
		ret = select(FD_SETSIZE, &fdset, NULL, NULL, &timeout);
		if (ret < 0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			if (errno == EINTR) {
				break;
			}
			continue;
		}

		if (ubus_connected) {
			if (FD_ISSET(ubus_ctx->sock.fd, &fdset)) {
				ubus_handle_event(ubus_ctx);
			}
			continue;
		}
		
		if (ubus_ctx) {
			if (ubus_reconnect(ubus_ctx, NULL) == 0) {
				printf("UBUS reconnected\n");
				ubus_connected = true;
				system_fd_set_cloexec(ubus_ctx->sock.fd);
			}
			continue;
		}
		
		ubus_ctx = ubus_connect(NULL);
		if (ubus_ctx) {
			ubus_ctx->connection_lost = ubus_connection_lost_cb;
			system_fd_set_cloexec(ubus_ctx->sock.fd);

			ret = 0;
			struct statd_rule *current_rule = statd_rule_get_head();
			while (current_rule) {
				struct ubus_event_handler *ubus_listener = get_ubus_event_handler(current_rule);
				if (ubus_listener) {
					ret |= ubus_register_event_handler(ubus_ctx, ubus_listener, statd_rule_get_filter(current_rule));
				}
				current_rule = statd_rule_get_next(current_rule);
			}
			if (ret == 0) {
				ubus_connected = true;
				printf("Connected to UBUS, id: %08x\n", ubus_ctx->local_id);
			} else {
				ubus_free(ubus_ctx);
				ubus_ctx = NULL;
			}
			continue;
		}
	}

	ubus_free(ubus_ctx); //Shut down UBUS connection
	printf("UBUS connection closed\n");

	statd_rule_destroy_all();
	return 0;
}
