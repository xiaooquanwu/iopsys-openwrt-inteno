#include <unistd.h>

#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "common.h"

static struct ubus_context *ctx;
static struct ubus_event_handler event_listener;
static struct blob_buf b;

static void
receive_event(struct ubus_context *ctx, struct ubus_event_handler *ev, const char *type, struct blob_attr *msg)
{
	char *str;
	const char *sta;

	str = blobmsg_format_json(msg, true);

	if(!strcmp(type, "wps")) {
		if(sta = json_parse_and_get(str, "status"))
			wps_event("status", sta);
		else if (sta = json_parse_and_get(str, "sta"))
			wps_event("sta", sta);
	}

	free(str);
}

void
ubus_listener()
{
	const char *ubus_socket = NULL;
	int ret;

	uloop_init();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return;
	}

	ubus_add_uloop(ctx);

	event_listener.cb = receive_event;
	ret = ubus_register_event_handler(ctx, &event_listener, "wps");
	if (ret)
		fprintf(stderr, "Couldn't register to router events\n");

	uloop_run();

	ubus_free(ctx);
	uloop_done();
}
