#include <syslog.h>
#include <string.h>
#include "button.h"
#include "log.h"
#include "server.h"

void sim_button_init(struct server_ctx *s_ctx);

/* static int sim_set_state(struct led_drv *drv, led_state_t state) */
/* { */
/* 	DBG(1, "state %x on %s\n", state, drv->name); */
/* 	return 0; */
/* } */

static button_state_t sim_get_state(struct button_drv *drv)
{
	DBG(1, "state for %s\n",  drv->name);
	return 0;
}

/* static int sim_set_color(struct led_drv *drv, led_color_t color) */
/* { */
/* 	DBG(1, "color %d on %s\n", color, drv->name); */
/* 	return 0; */
/* } */

/* static led_color_t sim_get_color(struct led_drv *drv) */
/* { */
/* 	DBG(1, "color for %s\n", drv->name); */
/* 	return 0; */
/* } */

static struct button_drv_func func = {
	.get_state = sim_get_state,
};

struct sim_data {
	int addr;
	int active;
	int state;
	struct button_drv button;
};


void sim_button_init(struct server_ctx *s_ctx) {

	LIST_HEAD(buttons);
	struct ucilist *node;

	DBG(1, "\n");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"sim_buttons", "buttons", &buttons);
	list_for_each_entry(node, &buttons, list) {
		struct sim_data *data;
		const char *s;

		DBG(1, "value = [%s]\n",node->val);

		data = malloc(sizeof(struct sim_data));
		memset(data,0,sizeof(struct sim_data));

		data->button.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "addr");
		DBG(1, "addr = [%s]\n", s);
		if (s){
			data->addr =  strtol(s,0,0);
		}

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "active");
		data->active = -1;
		if (s){
			if (!strncasecmp("hi",s,2))
				data->active =  1;
			else if (!strncasecmp("low",s,3))
				data->active =  0;

		}
		DBG(1, "active = %d\n", data->active);

		data->button.func = &func;
		data->button.priv = &data;
		button_add(&data->button);
	}
}
