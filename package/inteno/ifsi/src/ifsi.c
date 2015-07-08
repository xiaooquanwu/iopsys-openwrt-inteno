/*
 * ifsi -- Inteno functional script interface
 *
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * Author: dev@inteno.se
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <unistd.h>

#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>

#include <libubus.h>

static struct ubus_context *ctx;
static struct ubus_event_handler event_listener;
static struct blob_buf b;


static void receive_event(struct ubus_context *ctx, struct ubus_event_handler *ev,
			  const char *type, struct blob_attr *msg)
{
	char *str;
	uint32_t id;

	str = blobmsg_format_json(msg, true);
	fprintf(stdout, "I got %s event %s\n", type, str);
	free(str);
}

int main(int argc, char **argv)
{
	const char *ubus_socket = NULL;
	int ret;

	uloop_init();

	ctx = ubus_connect(ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return 1;
	}

	ubus_add_uloop(ctx);

	event_listener.cb = receive_event;
	ret = ubus_register_event_handler(ctx, &event_listener, "*");
	if (ret)
		fprintf(stderr, "Couldn't register to router events\n");

	uloop_run();

	ubus_free(ctx);
	uloop_done();

	return 0;
}

