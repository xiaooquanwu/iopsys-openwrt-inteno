#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <board.h>
#include "button.h"

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ucix.h"
#include "i2c.h"
#include "log.h"

#include "gpio.h"
#include "prox_px3220.h"

/* register names, from page 29, */
#define PX3220_SYSCONF		0x00
#define PX3220_IRQ_STATUS	0x01
#define PX3220_IRQ_CLEAR	0x02
#define PX3220_IR_DATA_LOW	0x0A
#define PX3220_IR_DATA_HIGH	0x0B
#define PX3220_PS_DATA_LOW	0x0E
#define PX3220_PS_DATA_HIGH	0x0F
#define PX3220_PS_LOW_THRESH_L	0x2A
#define PX3220_PS_LOW_THRESH_H	0x2B
#define PX3220_PS_HIGH_THRESH_L	0x2C
#define PX3220_PS_HIGH_THRESH_H	0x2D

#define IRQ_BUTTON 33

struct i2c_reg_tab {
	char addr;
	char value;
	char range;  /* if set registers starting from addr to addr+range will be set to the same value */
};

struct button_data {
	int addr;
	int state;
	struct button_drv button;
};

static const struct i2c_reg_tab i2c_init_tab_vox25[]={
        {PX3220_SYSCONF, 	  0x04, 0x00 },      /* Reset */
        {PX3220_SYSCONF, 	  0x02, 0x00 },      /* Power on IR */
        {PX3220_IRQ_STATUS,	  0x01, 0x00 },
        {0x20,           	  0x17, 0x00 },
        {0x23,           	  0x03, 0x00 },
        {PX3220_PS_LOW_THRESH_L,  0x00, 0x00 },
        {PX3220_PS_LOW_THRESH_H,  0x18, 0x00 },
        {PX3220_PS_HIGH_THRESH_L, 0x00, 0x00 },
        {PX3220_PS_HIGH_THRESH_H, 0x14, 0x00 },
};

int dev;
int shadow_proximity;

void do_init_tab(const struct i2c_reg_tab *tab, int len );


void do_init_tab(const struct i2c_reg_tab *tab, int len )
{
        int i;

	for (i = 0 ; i < len ; i++){
		int y;
		int ret;
		for ( y = 0 ; y <= tab[i].range; y++ ){
			DBG(3,"%s: addr %02X = %02X ",__func__,(unsigned char)tab[i].addr+y, (unsigned char)tab[i].value);
			ret = i2c_smbus_write_byte_data(dev, tab[i].addr+y, tab[i].value);
			if (ret < 0){
				perror("write to i2c dev\n");
			}
		}
	}

}

void px3220_check(void)
{
        int got_irq;
	unsigned short reg_low, reg_high, ps_val;

        shadow_proximity = 0;
        if (dev){
                got_irq = board_ioctl( BOARD_IOCTL_GET_GPIO, 0, 0, NULL, IRQ_BUTTON, 0);

                /* got_irq is active low */
                if (!got_irq) {
                        reg_low = i2c_smbus_read_byte_data(dev, PX3220_PS_DATA_LOW);

                        reg_high = i2c_smbus_read_byte_data(dev, PX3220_PS_DATA_HIGH);

                        ps_val = ((reg_high & 0x3F) << 4) | (reg_low & 0xF);

                        if (ps_val > 0x58)
                                shadow_proximity |= 0x1;
                        else
                                shadow_proximity |= 0x2;
                }
        }

        if (shadow_proximity)
                DBG(1,"shadow_proximity [%x]", shadow_proximity);
}

static button_state_t px3220_button_get_state(struct button_drv *drv)
{
 	struct button_data *p = (struct button_data *)drv->priv;

        if (p->addr == 0 ){
                if (shadow_proximity & 1 ) {
                        shadow_proximity &= ~0x1;
                        return p->state = BUTTON_PRESSED;
                }
        }

        if (p->addr == 1 ){
                if (shadow_proximity & 2 ) {
                        shadow_proximity &= ~0x2;
                        return p->state = BUTTON_PRESSED;
                }
        }

        p->state = BUTTON_RELEASED;
        return p->state;
}

static struct button_drv_func button_func = {
	.get_state = px3220_button_get_state,
};

void px3220_init(struct server_ctx *s_ctx) {

	const char *p;

	struct ucilist *node;
	LIST_HEAD(buttons);

	DBG(1, "");

	p = ucix_get_option(s_ctx->uci_ctx, "hw", "board", "hardware");
	if (p == 0){
		syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. I2C is not started\n",__func__);
		return;
	}

        /* Driver only existfor VOX25 board */
        if (strcmp("VOX25", p))
		return;

        /* open i2c device */
        dev = i2c_open_dev("/dev/i2c-0", 0x1E,
                           I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);
        if (dev < 0) {
		syslog(LOG_INFO,"%s: could not open i2c touch device\n",__func__);
		dev = 0;
		return;
	}

	do_init_tab(i2c_init_tab_vox25, sizeof(i2c_init_tab_vox25)/sizeof(struct i2c_reg_tab));


        /* read config file */
        ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"3220_buttons", "buttons", &buttons);
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

