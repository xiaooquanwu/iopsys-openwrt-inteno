#include <syslog.h>
#include <string.h>
#include "button.h"
#include "log.h"
#include "server.h"
#include "gpio.h"
#include <board.h>

void gpio_button_init(struct server_ctx *s_ctx);

struct gpio_button_data {
	int addr;
	int active;
	int state;
	struct button_drv button;
};

static button_state_t gpio_button_get_state(struct button_drv *drv)
{
//	DBG(1, "state for %s",  drv->name);
	struct gpio_button_data *p = (struct gpio_button_data *)drv->priv;
	int value;

	value = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, p->addr, 0);

	if(p->active)
		p->state = !!value;
	else
		p->state = !value;

	return p->state;
}

static struct button_drv_func func = {
	.get_state = gpio_button_get_state,
};

void gpio_button_init(struct server_ctx *s_ctx) {
	struct ucilist *node;
	LIST_HEAD(buttons);

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"gpio_buttons", "buttons", &buttons);
	list_for_each_entry(node, &buttons, list) {
		struct gpio_button_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct gpio_button_data));
		memset(data,0,sizeof(struct gpio_button_data));

		data->button.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "addr");
		DBG(1, "addr = [%s]", s);
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
		DBG(1, "active = %d", data->active);

		data->button.func = &func;
		data->button.priv = data;

		button_add(&data->button);
	}

	gpio_init();
}
