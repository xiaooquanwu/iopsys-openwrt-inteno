#include <syslog.h>
#include <string.h>
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

struct sim_data {
	int addr;
	led_color_t color;
	int state;
	int breading;
	struct led_drv led;
};


void sim_led_init(struct server_ctx *s_ctx) {

	LIST_HEAD(leds);
	struct ucilist *node;

	DBG(1, "\n");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"sim_leds", "leds", &leds);
	list_for_each_entry(node,&leds,list){
		struct sim_data *data;
		const char *s;

		DBG(1, "value = [%s]\n",node->val);

		data = malloc(sizeof(struct sim_data));
		memset(data,0,sizeof(struct sim_data));

		data->led.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "addr");
		DBG(1, "addr = [%s]\n", s);
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
		DBG(1, "color = [%s]=(%d)\n", s,data->color);

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "breading");
		DBG(1, "breading = [%s]\n", s);
		if (s){
			if (!strncasecmp("yes",s,3))
				data->breading = 1;
		}
		data->led.func = &func;
		data->led.priv = &data;
		led_add(&data->led);
	}
}
