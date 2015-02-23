#include "config.h"
#include <syslog.h>
#include "log.h"
#include "server.h"
#include "led.h"
#include "button.h"
#include "catv.h"
#include "sfp.h"
#include "touch_sx9512.h"

struct server_ctx server;

void sim_led_init(struct server_ctx *);
void sim_button_init(struct server_ctx *);

void gpio_led_init(struct server_ctx *);
void gpio_button_init(struct server_ctx *);

struct catv_handler *catv_h;
struct sfp_handler *sfp_h;

void server_start(struct uci_context *uci_ctx, struct ubus_context *ubus_ctx)

{
	DBG(1, "init server context.");
	server.uci_ctx = uci_ctx;
	server.ubus_ctx = ubus_ctx;

	DBG(1, "run init function for all hardware drivers.");

	sim_led_init(&server);
	sim_button_init(&server);

#ifdef HAVE_BOARD_H
	gpio_led_init(&server);
	gpio_button_init(&server);

	sx9512_init(&server);
#endif

	DBG(1, "connect generic buttons/leds to hardware drivers.");
	led_init(&server);
	button_init(&server);

	sfp_h = sfp_init(uci_ctx);
	if (sfp_h) {
		sfp_ubus_populate(sfp_h, ubus_ctx);
	}

	catv_h = catv_init(uci_ctx, "/dev/i2c-0", 0x50, 0x51);
	if (catv_h){
		catv_ubus_populate(catv_h, ubus_ctx);
	}

	DBG(1, "give control to uloop main loop.");
	uloop_run();
}
