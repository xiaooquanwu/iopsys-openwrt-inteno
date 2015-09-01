#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <board.h>
#include "button.h"
#include "led.h"
#include "log.h"
#include "server.h"

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ucix.h"
#include "i2c.h"

#include "touch_sx9512.h"
#include "gpio.h"

#define SX9512_IRQ_RESET		1<<7
#define SX9512_IRQ_TOUCH		1<<6
#define SX9512_IRQ_RELEASE		1<<5
#define SX9512_IRQ_NEAR			1<<4
#define SX9512_IRQ_FAR			1<<3
#define SX9512_IRQ_COM			1<<2
#define SX9512_IRQ_CONV			1<<1


struct i2c_reg_tab {
	char addr;
	char value;
	char range;  /* if set registers starting from addr to addr+range will be set to the same value */
};

struct i2c_touch{
	int dev;
	int shadow_irq;
	int shadow_touch;
	int shadow_proximity;
	int addr;
	int irq_button;
};

struct led_data {
	int addr;
	led_state_t state;
	struct led_drv led;
};

struct sx9512_list{
	struct list_head list;
	struct led_data *drv;
};

/* holds all leds that is configured to be used. needed for reset */
static LIST_HEAD(sx9512_leds);
static struct i2c_touch i2c_touch_current;	/* pointer to current active table */
static int sx9512_led_set_state(struct led_drv *drv, led_state_t state);
static led_state_t sx9512_led_get_state(struct led_drv *drv);
static int sx9512_set_state(struct led_data *p, led_state_t state);
extern struct uloop_timeout i2c_touch_reset_timer;


/* sx9512 needs a reset every 30 minutes. to recalibrate the touch detection */
#define I2C_RESET_TIME (1000 * 60 * 30) /* 30 min in ms */
static void sx9512_reset_handler(struct uloop_timeout *timeout);
struct uloop_timeout i2c_touch_reset_timer = { .cb = sx9512_reset_handler };

static void sx9512_reset_handler(struct uloop_timeout *timeout)
{
	struct list_head *i;
	//do_init_tab(i2c_touch_current);
	sx9512_reset_restore_led_state(i2c_touch_current.dev);

	list_for_each(i, &sx9512_leds) {
		struct sx9512_list *node = list_entry(i, struct sx9512_list, list);
		sx9512_set_state(node->drv, node->drv->state);
	}

	uloop_timeout_set(&i2c_touch_reset_timer, I2C_RESET_TIME);
}

/* set state regardless of previous state */
static int sx9512_set_state(struct led_data *p, led_state_t state)
{
	int ret;
	int bit = 1 << p->addr;

	ret = i2c_smbus_read_byte_data(i2c_touch_current.dev, SX9512_REG_LED_MAP2);
	if (ret < 0 )
		syslog(LOG_ERR, "Could not read from i2c device, LedMap2 register\n");

	if (state == ON)
		ret = ret | bit;
	else if (state == OFF)
		ret = ret & ~bit;
	else{
		syslog(LOG_ERR,"%s: Led %s: Set to not supported state %d\n",__func__, p->led.name, state);
		return -1;
	}

	p->state = state;

	ret = i2c_smbus_write_byte_data(i2c_touch_current.dev, SX9512_REG_LED_MAP2, ret);
	if (ret < 0 ) {
		syslog(LOG_ERR, "Could not write to i2c device, LedMap2 register\n");
		return -1;
	}
	return state;
}

/* set state if not same as current state  */
static int sx9512_led_set_state(struct led_drv *drv, led_state_t state)
{
	struct led_data *p = (struct led_data *)drv->priv;

	if (!i2c_touch_current.dev)
		return -1;

	if (p->addr > 7){
		DBG(1,"Led %s:with address %d outside range 0-7\n",drv->name, p->addr);
		return -1;
	}

	if (state == p->state ) {
		DBG(3,"skipping set");
		return state;
	}

	return sx9512_set_state(p, state);
}

static led_state_t sx9512_led_get_state(struct led_drv *drv)
{
	struct led_data *p = (struct led_data *)drv->priv;
	DBG(1, "state for %s",  drv->name);
	return p->state;
}

static struct led_drv_func led_func = {
	.set_state = sx9512_led_set_state,
	.get_state = sx9512_led_get_state,
};

struct button_data {
	int addr;
	int state;
	struct button_drv button;
};

void sx9512_check(void)
{
//	DBG(1, "state for %s",  drv->name);

	int got_irq = 0;
	int ret;

	if (!i2c_touch_current.dev)
		return;

	if (i2c_touch_current.irq_button) {
		int button;
		button = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, i2c_touch_current.irq_button, 0);
		if (button == 0)
			got_irq = 1;
	}
	if ( got_irq ) {

		ret = i2c_smbus_read_byte_data(i2c_touch_current.dev, SX9512_REG_IRQ_SRC);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, irq status register\n");
		i2c_touch_current.shadow_irq = ret;

		ret = i2c_smbus_read_byte_data(i2c_touch_current.dev, SX9512_REG_TOUCH_STATUS);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, touch register\n");
		i2c_touch_current.shadow_touch = ret;


		ret = i2c_smbus_read_byte_data(i2c_touch_current.dev, SX9512_REG_PROX_STATUS);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, proximity register\n");
		i2c_touch_current.shadow_proximity = ret;
	}
#if 0
    DEBUG_PRINT("%02x %02x %02x: irq ->",
                i2c_touch->shadow_irq ,
                i2c_touch->shadow_touch,
                i2c_touch->shadow_proximity);

    if (i2c_touch->shadow_irq & SX9512_IRQ_RESET )
        DEBUG_PRINT_RAW(" Reset ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_TOUCH )
        DEBUG_PRINT_RAW(" Touch ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_RELEASE )
        DEBUG_PRINT_RAW(" Release ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_NEAR )
        DEBUG_PRINT_RAW(" Near ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_FAR )
        DEBUG_PRINT_RAW(" Far ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_COM )
        DEBUG_PRINT_RAW(" Com ");
    if (i2c_touch->shadow_irq & SX9512_IRQ_CONV )
        DEBUG_PRINT_RAW(" Conv ");

    DEBUG_PRINT_RAW("\n");
#endif

	return ;
}
/*
  button address  0- 7 maps to touch event 0-7
  button address 8 proximity BL0 NEAR
  button address 9 proximity BL0 FAR

  return BUTTON_RELEASED = no action on this button
  return BUTTON_PRESSED = button pressed
  return -1 = error
*/

static button_state_t sx9512_button_get_state(struct button_drv *drv)
{
	struct button_data *p = (struct button_data *)drv->priv;
	int bit = 1 << p->addr;

	if (!i2c_touch_current.dev)
		return -1;

	if (p->addr < 8) {
		if ( bit & i2c_touch_current.shadow_touch ) {
			i2c_touch_current.shadow_touch = i2c_touch_current.shadow_touch & ~bit;
			return p->state = BUTTON_PRESSED;
		}

		/* if the button was already pressed and we don't have a release irq report it as still pressed */
		if( p->state == BUTTON_PRESSED){
			if (! (i2c_touch_current.shadow_irq & SX9512_IRQ_RELEASE) ) {
				return BUTTON_PRESSED;
			}
		}
		return p->state = BUTTON_RELEASED;

		/* proximity NEAR */
	}else if (p->addr == 8 ) {
		bit = 1<<7;
		if( i2c_touch_current.shadow_irq & SX9512_IRQ_NEAR ) {
			i2c_touch_current.shadow_irq &=  ~SX9512_IRQ_NEAR;
			if ( bit & i2c_touch_current.shadow_proximity ) {
				i2c_touch_current.shadow_proximity = i2c_touch_current.shadow_proximity & ~bit;
				return p->state = BUTTON_PRESSED;
			}
		}
		return BUTTON_RELEASED;

		/* proximity FAR */
	}else if (p->addr == 9) {
		if( i2c_touch_current.shadow_irq & SX9512_IRQ_FAR ) {
			i2c_touch_current.shadow_irq &=  ~SX9512_IRQ_FAR;
			return p->state = BUTTON_PRESSED;
		}
		return BUTTON_RELEASED;
	}else {
		DBG(1,"Button address out of range %d\n",p->addr);
		return BUTTON_RELEASED;
	}
}

static struct button_drv_func button_func = {
	.get_state = sx9512_button_get_state,
};


static void sx9512_button_init(struct server_ctx *s_ctx)
{
	struct ucilist *node;
	LIST_HEAD(buttons);

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"9512_buttons", "buttons", &buttons);
	list_for_each_entry(node, &buttons, list) {
		struct button_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct button_data));
		memset(data,0,sizeof(struct button_data));

		data->button.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->button.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s){
			data->addr =  strtol(s,0,0);
		}

		data->button.func = &button_func;
		data->button.priv = data;

		button_add(&data->button);
	}
}

static void sx9512_led_init(struct server_ctx *s_ctx) {

	LIST_HEAD(leds);
	struct ucilist *node;

	DBG(1, "");

	ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"9512_leds", "leds", &leds);
	list_for_each_entry(node,&leds,list){
		struct led_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct led_data));
		memset(data,0,sizeof(struct led_data));

		data->led.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s) {
			data->addr =  strtol(s,0,0);
		}

		data->led.func = &led_func;
		data->led.priv = data;
		led_add(&data->led);

		{ /* save leds in internal list, we need this for handling reset, we need to restore state */
			struct sx9512_list *ll = malloc(sizeof(struct sx9512_list));
			memset(ll, 0, sizeof( struct sx9512_list));
			ll->drv = data;
			list_add(&ll->list, &sx9512_leds);
		}


	}
}

void sx9512_handler_init(struct server_ctx *s_ctx)
{
	char *s, *sx9512_i2c_device;
	int i, fd, sx9512_i2c_address, sx9512_irq_pin, sx9512_active_capsense_channels, sx9512_active_led_channels;
	struct sx9512_reg_nvm nvm;
	struct list_head *il;
	
	if(!(sx9512_i2c_device = ucix_get_option(s_ctx->uci_ctx, "hw", "board", "sx9512_i2c_device"))) {
		DBG(0, "Error: option is missing: sx9512_i2c_device");
		return;
	}
	DBG(1, "sx9512_i2c_device = [%s]", sx9512_i2c_device);

	if(!(s=ucix_get_option(s_ctx->uci_ctx, "hw", "board", "sx9512_i2c_address"))) {
		DBG(0, "Warning: option is missing: sx9512_i2c_address, setting to default (%02X)", SX9512_I2C_ADDRESS);
		sx9512_i2c_address = SX9512_I2C_ADDRESS;
	} else
		sx9512_i2c_address = strtol(s,0,16);
	DBG(1, "sx9512_i2c_address = [%02X]", sx9512_i2c_address);

	if(!(s=ucix_get_option(s_ctx->uci_ctx, "hw", "board", "sx9512_irq_pin"))) {
		DBG(0, "Error: option is missing: sx9512_irq_pin");
		return;
	}
	sx9512_irq_pin = strtol(s,0,0);
	DBG(1, "sx9512_irq_pin = [%d]", sx9512_irq_pin);
	
	if(!(s=ucix_get_option(s_ctx->uci_ctx, "hw", "board", "sx9512_active_capsense_channels"))) {
		DBG(0, "Error: option is missing: sx9512_active_capsense_channels");
		return;
	}
	sx9512_active_capsense_channels = strtol(s,0,16);
	DBG(1, "sx9512_active_capsense_channels = [%02X]", sx9512_active_capsense_channels);
	
	if(!(s=ucix_get_option(s_ctx->uci_ctx, "hw", "board", "sx9512_active_led_channels"))) {
		DBG(0, "Error: option is missing: sx9512_active_led_channels");
		return;
	}
	sx9512_active_led_channels = strtol(s,0,16);
	DBG(1, "sx9512_active_led_channels = [%02X]", sx9512_active_led_channels);

	sx9512_reg_nvm_init_defaults(&nvm, sx9512_active_capsense_channels, sx9512_active_led_channels);
	
	LIST_HEAD(sx9512_init_regs);
	struct ucilist *node;
	ucix_get_option_list(s_ctx->uci_ctx, "hw","sx9512_init_regs", "regs", &sx9512_init_regs);
	list_for_each_entry(node,&sx9512_init_regs,list) {
		sx9512_reg_t reg;
		uint8_t val;
		int repeat;
		reg = strtol(node->val,0,16);
		if(sx9512_reg_reserved(reg)) {
			DBG(0, "Error: invalid sx9512 reg [%02X]", reg);
			return;
		}
		s = ucix_get_option(s_ctx->uci_ctx, "hw", node->val, "val");
		val = strtol(s,0,16);
		if(!(s = ucix_get_option(s_ctx->uci_ctx, "hw", node->val, "repeat")))
			repeat=1;
		else
			repeat=strtol(s,0,0);
		for(i=0;i<repeat;i++) {
			DBG(1, "sx9512_init_reg[%02X:%s=%02X]", reg, sx9512_reg_name(reg), val);
			((uint8_t *)&nvm)[reg-SX9512_REG_NVM_AREA_START] = val;
			reg++;
		}
	}

	if((fd = sx9512_init(sx9512_i2c_device, sx9512_i2c_address, &nvm))<1)
		return;
	i2c_touch_current.dev=fd;
	i2c_touch_current.addr=sx9512_i2c_address;
	i2c_touch_current.irq_button=sx9512_irq_pin;

	sx9512_button_init(s_ctx);
	sx9512_led_init(s_ctx);
	/* Force set of initial state for leds. */
	list_for_each(il, &sx9512_leds) {
		struct sx9512_list *node = list_entry(il, struct sx9512_list, list);
		sx9512_set_state(node->drv, node->drv->state);
	}
	/* start reset timer */
	uloop_timeout_set(&i2c_touch_reset_timer, I2C_RESET_TIME);
}

