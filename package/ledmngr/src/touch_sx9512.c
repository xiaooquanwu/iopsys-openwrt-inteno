#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <board.h>

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ucix.h"
#include "i2c.h"

#include "log.h"
#include "touch_sx9512.h"

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
     .name = "EG300",
     .irq_button = 1,
     .init_tab = i2c_init_tab_eg300,
     .init_tab_len = sizeof(i2c_init_tab_eg300)/sizeof(struct i2c_reg_tab),
    }
};

struct i2c_touch* sx9512_init(struct uci_context *uci_ctx) {
	return i2c_init(uci_ctx, "/dev/i2c-0", i2c_touch_list,
                        sizeof(i2c_touch_list)/sizeof(i2c_touch_list[0]));
}


extern struct uloop_timeout i2c_touch_reset_timer;

void sx9512_reset(struct i2c_touch *i2c_touch)
{
    do_init_tab(i2c_touch);
}

extern int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset);

int sx9512_check(struct i2c_touch *i2c_touch)
{
    int ret;
    int got_irq = 0;

    if (!i2c_touch || !i2c_touch->dev)
        return -1;

    if (i2c_touch->irq_button) {
        int button;
        button = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, i2c_touch->irq_button, 0);
        if (button == 0)
            got_irq = 1;
    }

    if ( got_irq ) {

        ret = i2c_smbus_read_byte_data(i2c_touch->dev, SX9512_IRQSRC);
        if (ret < 0 )
            syslog(LOG_ERR, "Could not read from i2c device, irq status register\n");
        i2c_touch->shadow_irq = ret;

        ret = i2c_smbus_read_byte_data(i2c_touch->dev, SX9512_TOUCHSTATUS);
        if (ret < 0 )
            syslog(LOG_ERR, "Could not read from i2c device, touch register\n");
        i2c_touch->shadow_touch = ret;


        ret = i2c_smbus_read_byte_data(i2c_touch->dev, SX9512_PROXSTATUS);
        if (ret < 0 )
            syslog(LOG_ERR, "Could not read from i2c device, proximity register\n");
        i2c_touch->shadow_proximity = ret;
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
    return 0;
}

/*
  button address  0- 7 maps to touch event 0-7
  button address 8 proximity BL0 NEAR
  button address 9 proximity BL0 FAR

  return 0 = no action on this button
  return 1 = button pressed
  return -1 = error
*/
int sx9512_check_button(struct button_config *bc, struct i2c_touch *i2c_touch) {

    int bit = 1 << bc->address;

    if (!i2c_touch || !i2c_touch->dev)
        return -1;

    if (bc->address < 8) {
        if ( bit & i2c_touch->shadow_touch ) {
            i2c_touch->shadow_touch = i2c_touch->shadow_touch & ~bit;
            return 1;
        }

        /* if the button was already pressed and we don't have a release irq report it as still pressed */
        if( bc->pressed_state ){
            if (! (i2c_touch->shadow_irq & SX9512_IRQ_RELEASE) ) {
                return 1;
            }
        }

        return 0;
    }else if (bc->address == 8 ) {
        bit = 1<<7;
        if( i2c_touch->shadow_irq & SX9512_IRQ_NEAR ) {
            i2c_touch->shadow_irq &=  ~SX9512_IRQ_NEAR;
            if ( bit & i2c_touch->shadow_proximity ) {
                i2c_touch->shadow_proximity = i2c_touch->shadow_proximity & ~bit;
                return 1;
            }
        }
        return 0;
    }else if (bc->address == 9) {
        if( i2c_touch->shadow_irq & SX9512_IRQ_FAR ) {
            i2c_touch->shadow_irq &=  ~SX9512_IRQ_FAR;
            return 1;
        }
        return 0;
    }else {
        DEBUG_PRINT("Button address out of range %d\n",bc->address);
        return 0;
    }
}

void sx9512_led_set( struct led_config* lc, int state){
    int ret;
    int bit = 1 << lc->address;

    if (!i2c_touch || !i2c_touch->dev)
        return;

    if (lc->address > 7){
        DEBUG_PRINT("Led %s:with address %d outside range 0-7\n",lc->name, lc->address);
        return;
    }

    ret = i2c_smbus_read_byte_data(i2c_touch->dev, SX9512_LEDMAP2);
    if (ret < 0 )
        syslog(LOG_ERR, "Could not read from i2c device, LedMap2 register\n");

    if (state == ON)
        ret = ret | bit;
    else if (state == OFF)
        ret = ret & ~bit;
    else{
        DEBUG_PRINT("Led %s: Set to not supported state %d\n",lc->name, state);
        return;
    }

    ret = i2c_smbus_write_byte_data(i2c_touch->dev, SX9512_LEDMAP2, ret);
    if (ret < 0 )
        syslog(LOG_ERR, "Could not read from i2c device, LedMap2 register\n");

}
