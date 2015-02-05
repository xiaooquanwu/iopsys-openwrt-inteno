#include <syslog.h>
#include "log.h"
#include "server.h"
#include "led.h"
#include "button.h"

struct server_ctx server;

void sim_led_init(struct server_ctx *);
void sim_button_init(struct server_ctx *);

void server_start(struct uci_context *uci_ctx, struct ubus_context *ubus_ctx)

{
	DBG(1, "init server context.\n");
	server.uci_ctx = uci_ctx;
	server.ubus_ctx = ubus_ctx;

	DBG(1, "run init function for all hardware drivers.\n");

	sim_led_init(&server);
	sim_button_init(&server);
//	sim_touch_init(&server);
//	sim_gpio_init(&server);

	DBG(1, "connect generic buttons/leds to hardware drivers.\n");
	led_init(&server);
	button_init(&server);

	DBG(1, "Start all timmers.\n");

	DBG(1, "give control to uloop main loop.\n");
	uloop_run();
}
