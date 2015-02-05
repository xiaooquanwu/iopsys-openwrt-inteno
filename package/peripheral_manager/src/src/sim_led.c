#include <syslog.h>
#include "led.h"
#include "log.h"
#include "server.h"

void sim_led_init(struct server_ctx *s_ctx);

static int set_state(struct led_drv *drv, led_state_t state)
{
	DBG(1, "state [%x]\n", state);
	return 0;
}

static led_state_t get_state(struct led_drv *drv)
{
	DBG(1, "\n");
	return 0;
}

static int set_color(struct led_drv *drv, led_color_t color)
{
	DBG(1, "color [%d]\n", color);
	return 0;
}

static led_color_t get_color(struct led_drv *drv)
{
	DBG(1, "\n");
	return 0;
}

static struct led_drv_func func = {
	.set_state = set_state,
	.get_state = get_state,
	.set_color = set_color,
	.get_color = get_color,
};

static struct led_drv led_drv = {
	.name = "sim_led_1",
	.func = &func,
};

void sim_led_init(struct server_ctx *s_ctx) {
	DBG(1, "\n");
	led_add(&led_drv);
}
