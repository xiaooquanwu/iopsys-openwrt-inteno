#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <board.h>
#include "led.h"
#include "log.h"
#include "server.h"


void gpio_led_init(struct server_ctx *s_ctx);

typedef enum {
	UNKNOWN,
	LOW,
	HI,
} active_t;


struct gpio_data {
	int addr;
	active_t active;
	int state;
	struct led_drv led;
};

static int brcmboard = -1;

static void open_ioctl() {

    brcmboard = open("/dev/brcmboard", O_RDWR);
    if ( brcmboard == -1 ) {
	    DBG(1,"failed to open: /dev/brcmboard\n");
        return;
    }
    DBG(1, "fd %d allocated\n", brcmboard);
    return;
}

int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
	BOARD_IOCTL_PARMS IoctlParms = {0};
	IoctlParms.string = string_buf;
	IoctlParms.strLen = string_buf_len;
	IoctlParms.offset = offset;
	IoctlParms.action = action;
	IoctlParms.buf    = "";
	if ( ioctl(brcmboard, ioctl_id, &IoctlParms) < 0 ) {
		syslog(LOG_INFO, "ioctl: %d failed", ioctl_id);
		exit(1);
	}
	return IoctlParms.result;
}

static int gpio_set_state(struct led_drv *drv, led_state_t state)
{
	struct gpio_data *p = (struct gpio_data *)drv->priv;
	int active;

	if (state == OFF) {
		if (p->active == HI)
			active = 0;
		else if (p->active == LOW)
			active = 1;

	}else if (state == ON) {
		if (p->active == HI)
			active = 1;
		else if (p->active == LOW)
			active = 0;
	}

	p->state = state;

	board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, p->addr, active);

	return p->state;
}

static led_state_t gpio_get_state(struct led_drv *drv)
{
	struct gpio_data *p = (struct gpio_data *)drv->priv;
	DBG(1, "state for %s",  drv->name);

	return p->state;
}

static struct led_drv_func func = {
	.set_state = gpio_set_state,
	.get_state = gpio_get_state,
};

void gpio_led_init(struct server_ctx *s_ctx) {

	LIST_HEAD(leds);
	struct ucilist *node;

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"gpio_leds", "leds", &leds);
	list_for_each_entry(node,&leds,list){
		struct gpio_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct gpio_data));
		memset(data,0,sizeof(struct gpio_data));

		data->led.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s) {
			data->addr =  strtol(s,0,0);
		}

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "active");
		DBG(1, "active = [%s]", s);
		if (s) {
			data->active = UNKNOWN;
			if (!strncasecmp("hi",s,3))
				data->active = HI;
			if (!strncasecmp("low",s,3))
				data->active = LOW;
		}
		data->led.func = &func;
		data->led.priv = data;
		led_add(&data->led);
	}
	open_ioctl();
}
