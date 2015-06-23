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

/* register names, from page 29, */
#define SX9512_IRQSRC			0
#define SX9512_TOUCHSTATUS		1
#define SX9512_PROXSTATUS		2
#define SX9512_LEDMAP1			0xC
#define SX9512_LEDMAP2			0xD

#define SX9512_IRQ_RESET		1<<7
#define SX9512_IRQ_TOUCH		1<<6
#define SX9512_IRQ_RELEASE		1<<5
#define SX9512_IRQ_NEAR			1<<4
#define SX9512_IRQ_FAR			1<<3
#define SX9512_IRQ_COM			1<<2
#define SX9512_IRQ_CONV			1<<1

/* CG300 config:

   BL0: Proximity
   BL1: Wireless button
   BL2: WPS button, WPS LED
   BL3: Dect button, Dect LED
   BL4: Internet green LED
   BL5: Internet red LED
   BL6, BL7: Unused.
*/

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
	const struct i2c_reg_tab *init_tab;
	int init_tab_len;
	const char *name;
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

static struct i2c_touch *i2c_touch_current;	/* pointer to current active table */

static void do_init_tab( struct i2c_touch *i2c_touch);
static struct i2c_touch * i2c_init(struct uci_context *uci_ctx, const char* i2c_dev_name, struct i2c_touch* i2c_touch_list, int len);

static int sx9512_led_set_state(struct led_drv *drv, led_state_t state);
static led_state_t sx9512_led_get_state(struct led_drv *drv);
static int sx9512_set_state(struct led_data *p, led_state_t state);


/*addr,value,range*/
static const struct i2c_reg_tab i2c_init_tab_cg300[]={
    {0xFF, 0xDE, 0x00 },      /* Reset chip */
    {0xFF, 0x00, 0x00 },      /* Reset chip */

    {0x04, 0x00, 0x00 },      /* NVM Control */
    {0x07, 0x00, 0x00 },      /* SPO2, set as interrupt  */
    {0x08, 0x00, 0x00 },      /* Power key ctrl */
    {0x09, 0x78, 0x00 },      /* Irq MASK */
    {0x0C, 0x01, 0x00 },      /* LED map 1, BL0 (why?) */
    {0x0D, 0x3c, 0x00 },      /* LED map 2 BL2 -> BL5*/
    {0x0E, 0x10, 0x00 },      /* LED Pwm Frq */
    {0x0F, 0x00, 0x00 },      /* LED Mode */
    {0x10, 0xFF, 0x00 },      /* Led Idle LED on */
    {0x11, 0x00, 0x00 },      /* Led 1 off delay */
    {0x12, 0xFF, 0x00 },      /* Led 1 on */
    {0x13, 0x00, 0x00 },      /* Led 1 fade */
    {0x14, 0xFF, 0x00 },      /* Led 2 on   */
    {0x15, 0x00, 0x00 },      /* Led 2 fade */
    {0x16, 0xFF, 0x00 },      /* Led Pwr Idle */
    {0x17, 0xFF, 0x00 },      /* Led Pwr On */
    {0x18, 0x00, 0x00 },      /* Led Pwr Off */
    {0x19, 0x00, 0x00 },      /* Led Pwr fade */
    {0x1A, 0x00, 0x00 },      /* Led Pwr On Pw */
    {0x1B, 0x00, 0x00 },      /* Disable BL7 as power button */
    {0x1E, 0x0F, 0x00 },      /* Cap sens enabled, bl0-bl3 */
    {0x1F, 0x43, 0x00 },      /* Cap sens BL0 */
    {0x20, 0x41, 0x07 },      /* Cap sens range  20-27 BL1->BL7 */
    {0x28, 0x02, 0x00 },      /* Cap sens thresh BL 0  */
    {0x29, 0x04, 0x07 },      /* Cap sens thresh 28-30 */
    {0x31, 0x54, 0x00 },      /* Cap sens Op */
    {0x32, 0x70, 0x00 },      /* Cap Sens Mode, filter 1-1/8, report all */
    {0x33, 0x01, 0x00 },      /* Cap Sens Debounce */
    {0x34, 0x80, 0x00 },      /* Cap Sens Neg Comp Thresh */
    {0x35, 0x80, 0x00 },      /* Cap Sens Pos Comp Thresh */
    {0x36, 0x17, 0x00 },      /* Cap Sens Pos Filt, hyst 2, debounce 4, 1-1/128 */
    {0x37, 0x15, 0x00 },      /* Cap Sens Neg Filt, hyst 2, debounce 4, 1-1/32 */
    {0x38, 0x00, 0x00 },      /* Cap Sens */
    {0x39, 0x00, 0x00 },      /* Cap Sens Frame Skip  */
    {0x3A, 0x00, 0x00 },      /* Cap Sens Misc  */
    {0x3B, 0x00, 0x00 },      /* Prox Comb Chan Mask */
    {0x3E, 0xFF, 0x00 },      /* SPO Chan Map */
    {0x00, 0x04, 0x00 },      /* Trigger compensation */
};

/* CG301 config:

   BL0: Proximity, Status white LED
   BL1: Service button, Service white LED
   BL2: Service red LED
   BL3: Pair button, Pair white LED
   BL4: Cancel button, Cancel red LED
   BL5: Communication button, Communication white LED
   BL6: Communication red LED
   BL7: Unused.
*/

static const struct i2c_reg_tab i2c_init_tab_cg301[]={
    {0xFF, 0xDE, 0x00 },      /* Reset chip */
    {0xFF, 0x00, 0x00 },      /* Reset chip */

    {0x04, 0x00, 0x00 },      /* NVM Control */
    {0x07, 0x00, 0x00 },      /* SPO2, set as interrupt  */
    {0x08, 0x00, 0x00 },      /* Power key ctrl */
    {0x09, 0x78, 0x00 },      /* Irq MASK, touch+release+near+far */
    {0x0C, 0x00, 0x00 },      /* LED map 1, none */
    {0x0D, 0x7f, 0x00 },      /* LED map 2, BL0 -> BL6 */
    {0x0E, 0x10, 0x00 },      /* LED Pwm Frq */
    {0x0F, 0x00, 0x00 },      /* LED Mode */
    {0x10, 0xFF, 0x00 },      /* Led Idle LED on */
    {0x11, 0x00, 0x00 },      /* Led 1 off delay */
    {0x12, 0xFF, 0x00 },      /* Led 1 on */
    {0x13, 0x00, 0x00 },      /* Led 1 fade */
    {0x14, 0xFF, 0x00 },      /* Led 2 on   */
    {0x15, 0x00, 0x00 },      /* Led 2 fade */
    {0x16, 0xFF, 0x00 },      /* Led Pwr Idle */
    {0x17, 0xFF, 0x00 },      /* Led Pwr On */
    {0x18, 0x00, 0x00 },      /* Led Pwr Off */
    {0x19, 0x00, 0x00 },      /* Led Pwr fade */
    {0x1A, 0x00, 0x00 },      /* Led Pwr On Pw */
    {0x1B, 0x00, 0x00 },      /* Disable BL7 as power button */
    {0x1E, 0x3b, 0x00 },      /* Cap sens enabled, bl0,bl1,bl3-bl5 */
    {0x1F, 0x43, 0x00 },      /* Cap sens range BL0 */
    {0x20, 0x41, 0x07 },      /* Cap sens range BL1->BL7 [20-27] */
    {0x28, 0x02, 0x00 },      /* Cap sens thresh BL0  */
    {0x29, 0x04, 0x07 },      /* Cap sens thresh BL1->BL7 [29-30] */
    {0x31, 0x54, 0x00 },      /* Cap sens Op */
    {0x32, 0x70, 0x00 },      /* Cap Sens Mode, filter 1-1/8, report all */
    {0x33, 0x01, 0x00 },      /* Cap Sens Debounce */
    {0x34, 0x80, 0x00 },      /* Cap Sens Neg Comp Thresh */
    {0x35, 0x80, 0x00 },      /* Cap Sens Pos Comp Thresh */
    {0x36, 0x17, 0x00 },      /* Cap Sens Pos Filt, hyst 2, debounce 4, 1-1/128 */
    {0x37, 0x15, 0x00 },      /* Cap Sens Neg Filt, hyst 2, debounce 4, 1-1/32 */
    {0x38, 0x00, 0x00 },      /* Cap Sens */
    {0x39, 0x00, 0x00 },      /* Cap Sens Frame Skip  */
    {0x3A, 0x00, 0x00 },      /* Cap Sens Misc  */
    {0x3B, 0x00, 0x00 },      /* Prox Comb Chan Mask */
    {0x3E, 0xFF, 0x00 },      /* SPO Chan Map */
    {0x00, 0x04, 0x00 },      /* Trigger compensation */
};

/* EG300 config:

   BL0: Proximity, WAN green LED
   BL1: Wireless button, WAN yellow LED
   BL2: WPS button, WPS LED
   BL3: Dect button, Dect LED
   BL4: Internet green LED
   BL5: Internet red LED
   BL6: Ethernet LED
   BL7: Voice LED

   Only the led 1 and led2 maps differ from CG300.
*/

static const struct i2c_reg_tab i2c_init_tab_eg300[]={
    {0xFF, 0xDE, 0x00 },      /* Reset chip */
    {0xFF, 0x00, 0x00 },      /* Reset chip */

    {0x04, 0x00, 0x00 },      /* NVM Control */
    {0x07, 0x00, 0x00 },      /* SPO2, set as interrupt  */
    {0x08, 0x00, 0x00 },      /* Power key ctrl */
    {0x09, 0x78, 0x00 },      /* Irq MASK */
    {0x0C, 0x00, 0x00 },      /* LED map 1, none */
    {0x0D, 0xff, 0x00 },      /* LED map 2, all */
    {0x0E, 0x10, 0x00 },      /* LED Pwm Frq */
    {0x0F, 0x00, 0x00 },      /* LED Mode */
    {0x10, 0xFF, 0x00 },      /* Led Idle LED on */
    {0x11, 0x00, 0x00 },      /* Led 1 off delay */
    {0x12, 0xFF, 0x00 },      /* Led 1 on */
    {0x13, 0x00, 0x00 },      /* Led 1 fade */
    {0x14, 0xFF, 0x00 },      /* Led 2 on   */
    {0x15, 0x00, 0x00 },      /* Led 2 fade */
    {0x16, 0xFF, 0x00 },      /* Led Pwr Idle */
    {0x17, 0xFF, 0x00 },      /* Led Pwr On */
    {0x18, 0x00, 0x00 },      /* Led Pwr Off */
    {0x19, 0x00, 0x00 },      /* Led Pwr fade */
    {0x1A, 0x00, 0x00 },      /* Led Pwr On Pw */
    {0x1B, 0x00, 0x00 },      /* Disable BL7 as power button */
    {0x1E, 0x0F, 0x00 },      /* Cap sens enabled, bl0-bl3 */
    {0x1F, 0x43, 0x00 },      /* Cap sens BL0 */
    {0x20, 0x43, 0x07 },      /* Cap sens range  20-27 BL1->BL7 */
    {0x28, 0x02, 0x00 },      /* Cap sens thresh BL 0  */
    {0x29, 0x04, 0x07 },      /* Cap sens thresh 28-30 */
    {0x31, 0x54, 0x00 },      /* Cap sens Op */
    {0x32, 0x70, 0x00 },      /* Cap Sens Mode, filter 1-1/8, report all */
    {0x33, 0x01, 0x00 },      /* Cap Sens Debounce */
    {0x34, 0x80, 0x00 },      /* Cap Sens Neg Comp Thresh */
    {0x35, 0x80, 0x00 },      /* Cap Sens Pos Comp Thresh */
    {0x36, 0x17, 0x00 },      /* Cap Sens Pos Filt, hyst 2, debounce 4, 1-1/128 */
    {0x37, 0x15, 0x00 },      /* Cap Sens Neg Filt, hyst 2, debounce 4, 1-1/32 */
    {0x38, 0x00, 0x00 },      /* Cap Sens */
    {0x39, 0x00, 0x00 },      /* Cap Sens Frame Skip  */
    {0x3A, 0x00, 0x00 },      /* Cap Sens Misc  */
    {0x3B, 0x00, 0x00 },      /* Prox Comb Chan Mask */
    {0x3E, 0xFF, 0x00 },      /* SPO Chan Map */
    {0x00, 0x04, 0x00 },      /* Trigger compensation */
};

static struct i2c_touch i2c_touch_list[] = {
	{.addr = 0x2b,
	 .name = "CG300",
	 .irq_button = 1,
	 .init_tab = i2c_init_tab_cg300,
	 .init_tab_len = sizeof(i2c_init_tab_cg300)/sizeof(struct i2c_reg_tab),
	},

	{.addr = 0x2b,
	 .name = "CG301",
	 .irq_button = 1,
	 .init_tab = i2c_init_tab_cg301,
	 .init_tab_len = sizeof(i2c_init_tab_cg301)/sizeof(struct i2c_reg_tab),
	},

	{.addr = 0x2b,
	 .name = "EG300",
	 .irq_button = 1,
	 .init_tab = i2c_init_tab_eg300,
	 .init_tab_len = sizeof(i2c_init_tab_eg300)/sizeof(struct i2c_reg_tab),
	}
};

static void do_init_tab( struct i2c_touch *i2c_touch)
{
	const struct i2c_reg_tab *tab;
	int i;

	tab = i2c_touch->init_tab;

	for (i = 0 ; i < i2c_touch->init_tab_len ; i++){
		int y;
		int ret;
		for ( y = 0 ; y <= tab[i].range; y++ ){
			DBG(3,"%s: addr %02X = %02X ",__func__,(unsigned char)tab[i].addr+y, (unsigned char)tab[i].value);
			ret = i2c_smbus_write_byte_data(i2c_touch->dev, tab[i].addr+y, tab[i].value);
			if (ret < 0){
				perror("write to i2c dev\n");
			}
		}
	}
//  dump_i2c(i2c_touch->dev,0,13);
}


static struct i2c_touch * i2c_init(struct uci_context *uci_ctx, const char* i2c_dev_name, struct i2c_touch* touch_list, int len)
{
	const char *p;
	int i;
	struct i2c_touch *i2c_touch;

	p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
	if (p == 0){
		syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. I2C is not started\n",__func__);
		return 0;
	}

	/* Here we match the hardware name to a init table, and get the
	   i2c chip address */
	i2c_touch = NULL;
	for (i = 0; i < len; i++) {
		if (!strcmp(touch_list[i].name, p)) {
			DBG(1,"I2C hardware platform %s found.\n", p);
			i2c_touch = &touch_list[i];
			break;
		}
	}

	if (!i2c_touch) {
		DBG(1,"No I2C hardware found: %s.\n", p);
		return 0;
	}

	i2c_touch->dev = i2c_open_dev(i2c_dev_name, i2c_touch->addr,
				      I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);

	if (i2c_touch->dev < 0) {
		syslog(LOG_INFO,"%s: could not open i2c touch device\n",__func__);
		i2c_touch->dev = 0;
		return 0;
	}

	DBG(1,"Opened device and selected address %x \n", i2c_touch->addr);

	do_init_tab(i2c_touch);

	return i2c_touch;
}

extern struct uloop_timeout i2c_touch_reset_timer;


/* sx9512 needs a reset every 30 minutes. to recalibrate the touch detection */
#define I2C_RESET_TIME (1000 * 60 * 30) /* 30 min in ms */
static void sx9512_reset_handler(struct uloop_timeout *timeout);
struct uloop_timeout i2c_touch_reset_timer = { .cb = sx9512_reset_handler };

static void sx9512_reset_handler(struct uloop_timeout *timeout)
{
	struct list_head *i;
	do_init_tab(i2c_touch_current);

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

	ret = i2c_smbus_read_byte_data(i2c_touch_current->dev, SX9512_LEDMAP2);
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

	ret = i2c_smbus_write_byte_data(i2c_touch_current->dev, SX9512_LEDMAP2, ret);
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

	if (!i2c_touch_current || !i2c_touch_current->dev)
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

	if (!i2c_touch_current || !i2c_touch_current->dev)
		return;

	if (i2c_touch_current->irq_button) {
		int button;
		button = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, i2c_touch_current->irq_button, 0);
		if (button == 0)
			got_irq = 1;
	}
	if ( got_irq ) {

		ret = i2c_smbus_read_byte_data(i2c_touch_current->dev, SX9512_IRQSRC);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, irq status register\n");
		i2c_touch_current->shadow_irq = ret;

		ret = i2c_smbus_read_byte_data(i2c_touch_current->dev, SX9512_TOUCHSTATUS);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, touch register\n");
		i2c_touch_current->shadow_touch = ret;


		ret = i2c_smbus_read_byte_data(i2c_touch_current->dev, SX9512_PROXSTATUS);
		if (ret < 0 )
			syslog(LOG_ERR, "Could not read from i2c device, proximity register\n");
		i2c_touch_current->shadow_proximity = ret;
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

  return RELEASED = no action on this button
  return PRESSED = button pressed
  return -1 = error
*/

static button_state_t sx9512_button_get_state(struct button_drv *drv)
{
	struct button_data *p = (struct button_data *)drv->priv;
	int bit = 1 << p->addr;

	if (!i2c_touch_current || !i2c_touch_current->dev)
		return -1;

	if (p->addr < 8) {
		if ( bit & i2c_touch_current->shadow_touch ) {
			i2c_touch_current->shadow_touch = i2c_touch_current->shadow_touch & ~bit;
			p->state = PRESSED;
			return PRESSED;
		}

		/* if the button was already pressed and we don't have a release irq report it as still pressed */
		if( p->state == PRESSED){
			if (! (i2c_touch_current->shadow_irq & SX9512_IRQ_RELEASE) ) {
				return PRESSED;
			}
		}
		p->state = RELEASED;
		return RELEASED;

		/* proximity NEAR */
	}else if (p->addr == 8 ) {
		bit = 1<<7;
		if( i2c_touch_current->shadow_irq & SX9512_IRQ_NEAR ) {
			i2c_touch_current->shadow_irq &=  ~SX9512_IRQ_NEAR;
			if ( bit & i2c_touch_current->shadow_proximity ) {
				i2c_touch_current->shadow_proximity = i2c_touch_current->shadow_proximity & ~bit;
				p->state = PRESSED;
				return PRESSED;
			}
		}
		return RELEASED;

		/* proximity FAR */
	}else if (p->addr == 9) {
		if( i2c_touch_current->shadow_irq & SX9512_IRQ_FAR ) {
			i2c_touch_current->shadow_irq &=  ~SX9512_IRQ_FAR;
			p->state = PRESSED;
			return PRESSED;
		}
		return RELEASED;
	}else {
		DBG(1,"Button address out of range %d\n",p->addr);
		return RELEASED;
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

void sx9512_init(struct server_ctx *s_ctx) {

	struct list_head *i;

	DBG(1, "");

	i2c_touch_current = i2c_init(s_ctx->uci_ctx,
			     "/dev/i2c-0",
			     i2c_touch_list,
			     sizeof(i2c_touch_list)/sizeof(i2c_touch_list[0]));

	if (i2c_touch_current != 0) {

		sx9512_button_init(s_ctx);
		sx9512_led_init(s_ctx);

		/* Force set of initial state for leds. */
		list_for_each(i, &sx9512_leds) {
			struct sx9512_list *node = list_entry(i, struct sx9512_list, list);
			sx9512_set_state(node->drv, node->drv->state);
		}

		/* start reset timer */
		uloop_timeout_set(&i2c_touch_reset_timer, I2C_RESET_TIME);
	}
}

