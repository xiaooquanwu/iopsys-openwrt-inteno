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
#include "gpio.h"


void gpio_led_init(struct server_ctx *s_ctx);

typedef enum {
	UNKNOWN,
	LOW,
	HI,
} active_t;

typedef enum {
	MODE_UNKNOWN,
	DIRECT,
	SHIFTREG2,
	SHIFTREG3,
} gpio_mode_t;

struct gpio_data {
	int addr;
	active_t active;
	int state;
	gpio_mode_t mode;
	struct led_drv led;
};

#define SR_MAX 16
static int shift_register_state[SR_MAX];

static void shift_register3_set(int address, int bit_val) {
	int i;

	if (address>=SR_MAX-1) {
		DBG(1,"address index %d too large\n", address);
		return;
	}

	// Update internal register copy
	shift_register_state[address] = bit_val;

	// pull down shift register load (load gpio 23)
	board_ioctl(BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 0);

	// clock in bits
	for (i=0 ; i<SR_MAX ; i++) {
		//set clock low
		board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 0);
		//place bit on data line
		board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 1, shift_register_state[SR_MAX-1-i]);
		//set clock high
		board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 0, 1);
	}

	// issue shift register load
	board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, 23, 1);
}

static int gpio_set_state(struct led_drv *drv, led_state_t state)
{
	struct gpio_data *p = (struct gpio_data *)drv->priv;
	int bit_val = 0;

	if (state == OFF) {
		if (p->active == HI)
			bit_val = 0;
		else if (p->active == LOW)
			bit_val = 1;

	}else if (state == ON) {
		if (p->active == HI)
			bit_val = 1;
		else if (p->active == LOW)
			bit_val = 0;
	}

	p->state = state;

	switch (p->mode) {
	case DIRECT :
		board_ioctl( BOARD_IOCTL_SET_GPIO, 0, 0, NULL, p->addr, bit_val);
		break;
	case SHIFTREG2:
		board_ioctl( BOARD_IOCTL_LED_CTRL, 0, 0, NULL, p->addr, bit_val);
		break;
	case SHIFTREG3:
		shift_register3_set(p->addr, bit_val);
		break;
	default:
		DBG(1,"access mode not supported [%d]", p->mode);
	}

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

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "mode");
		DBG(1, "mode = [%s]", s);
		if (s) {

			if (!strncasecmp("direct",s,3))
				data->mode =  DIRECT;
			else if (!strncasecmp("sr",s,5))
				data->mode =  SHIFTREG2;
			else if (!strncasecmp("csr",s,4))
				data->mode =  SHIFTREG3;
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
	gpio_open_ioctl();
}
