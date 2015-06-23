#include <syslog.h>
#include <string.h>
#include "led.h"
#include "log.h"
#include "server.h"

void sim_led_init(struct server_ctx *s_ctx);

static int sim_set_state(struct led_drv *drv, led_state_t state)
{
	DBG(1, "state %x on %s", state, drv->name);
	return 0;
}

static led_state_t sim_get_state(struct led_drv *drv)
{
	DBG(1, "state for %s",  drv->name);
	return 0;
}

static int sim_set_color(struct led_drv *drv, led_color_t color)
{
	DBG(1, "color %d on %s", color, drv->name);
	return 0;
}

static led_color_t sim_get_color(struct led_drv *drv)
{
	DBG(1, "color for %s", drv->name);
	return 0;
}

static struct led_drv_func func = {
	.set_state = sim_set_state,
	.get_state = sim_get_state,
	.set_color = sim_set_color,
	.get_color = sim_get_color,
};

struct sim_data {
	int addr;
	led_color_t color;
	int state;
	int pulsing;
	struct led_drv led;
};

void sim_led_init(struct server_ctx *s_ctx) {

	LIST_HEAD(leds);
	struct ucilist *node;

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"sim_leds", "leds", &leds);
	list_for_each_entry(node,&leds,list){
		struct sim_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct sim_data));
		memset(data,0,sizeof(struct sim_data));

		data->led.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s){
			data->addr =  strtol(s,0,0);
		}

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "color");
		if (s){
			if (!strncasecmp("red",s,3))
				data->color =  RED;
			else if (!strncasecmp("green",s,5))
				data->color =  GREEN;
			else if (!strncasecmp("blue",s,4))
				data->color =  BLUE;
			else if (!strncasecmp("yellow",s,6))
				data->color =  YELLOW;
			else if (!strncasecmp("white",s,5))
				data->color =  WHITE;
		}
		DBG(1, "color = [%s]=(%d)", s,data->color);

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "pulsing");
		DBG(1, "pulsing = [%s]", s);
		if (s){
			if (!strncasecmp("yes",s,3))
				data->pulsing = 1;
		}
		data->led.func = &func;
		data->led.priv = data;
		led_add(&data->led);
	}
}
