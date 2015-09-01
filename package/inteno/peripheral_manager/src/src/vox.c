#include <syslog.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <board.h>
#include "log.h"
#include "server.h"
#include "led.h"
#include "gpio.h"

#define SPI_SLAVE_SELECT 1

struct vox_data {
	int		addr;
	led_state_t	state;
	int		brightness;
	struct led_drv led;
};

void vox_init(struct server_ctx *s_ctx);

static int vox_set_state(struct led_drv *drv, led_state_t state)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
        char spi_data[6] = {0,0,0,0,0,0};

        if (p->state == state)
                    return state;

	memset(spi_data, 0, 6);

        spi_data[0] = p->addr;

	if (state == ON) {
                spi_data[1] = 1;
		spi_data[2] = 0x0;
		spi_data[3] = 0x0;
		spi_data[4] = 0x0;
		spi_data[4] = 0x0;
	} else if(state == PULSING) {
                spi_data[1] = 3;
		spi_data[2] = 0xa0;
	} else if(state == FLASH_SLOW) {
                spi_data[1] = 2;
		spi_data[3] = 0x95;
	} else if(state == FLASH_FAST) {
                spi_data[1] = 2;
		spi_data[3] = 0x20;
	}

        DBG(2,"vox_set_state %x %x %x %x",spi_data[0],spi_data[1],spi_data[2],spi_data[3]);
	board_ioctl(BOARD_IOCTL_SPI_WRITE, SPI_SLAVE_SELECT, 0, spi_data, 6, 0);

	p->state = state;
        return state;
}

static led_state_t vox_get_state(struct led_drv *drv)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
	return p->state;
}

/* input  brightness is in %. 0-100      */
/* internal brightness is 5 steps. 0-4   */
/*
  step, level percent mapping.
  0	0 -> 20
  1	21 -> 40
  2	41 -> 60
  3	61 -> 80
  4	81 -> 100

*/

static 	int vox_set_brightness(struct led_drv *drv, int level)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
	int new = (level * 5)/101;    /* really level/(101/5) */
        char spi_data[6] = {0,0,0,0,0,0};

	if (new == p->brightness)
		return level;

	p->brightness = new;

	memset(spi_data, 0, 6);

        spi_data[0] = p->addr;
        spi_data[1] = 6;
	spi_data[2] = p->brightness;

        DBG(2,"vox_set_state %x %x %x %x",spi_data[0],spi_data[1],spi_data[2],spi_data[3]);
	board_ioctl(BOARD_IOCTL_SPI_WRITE, SPI_SLAVE_SELECT, 0, spi_data, 6, 0);

	return level;
}

static	int vox_get_brightness(struct led_drv *drv)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
	return p->brightness * (100/5);
}

static int vox_support(struct led_drv *drv, led_state_t state)
{
	switch (state) {

	case OFF:
	case ON:
	case FLASH_SLOW:
	case FLASH_FAST:
	case PULSING:
		return 1;
		break;

	default:
		return 0;
	}
	return 0;
}

static struct led_drv_func func = {
	.set_state       = vox_set_state,
	.get_state       = vox_get_state,
	.set_brightness = vox_set_brightness,
	.get_brightness = vox_get_brightness,
	.support         = vox_support,
};

void vox_init(struct server_ctx *s_ctx) {
 	LIST_HEAD(leds);
	struct ucilist *node;
	int register_spi = 0;

        DBG(1, "");

        ucix_get_option_list(s_ctx->uci_ctx, "hw" ,"vox_leds", "leds", &leds);
	list_for_each_entry(node,&leds,list){
		struct vox_data *data;
		const char *s;

		DBG(1, "value = [%s]",node->val);

		data = malloc(sizeof(struct vox_data));
		memset(data,0,sizeof(struct vox_data));

		data->led.name = node->val;

		s = ucix_get_option(s_ctx->uci_ctx, "hw" , data->led.name, "addr");
		DBG(1, "addr = [%s]", s);
		if (s) {
			data->addr =  strtol(s,0,0);
		}else
                        syslog(LOG_ERR,"vox_led config needs addr option\n");

		data->led.func = &func;
		data->led.priv = data;
                data->state = NEED_INIT;
		data->brightness = 4;
		led_add(&data->led);
		register_spi = 1;
	}

	/* if config entries for vox leds exist register the spi as used. */
	if(register_spi) {
		/* arg 4 is the spi mode encoded in a string pointer */
		/* mode is decribed i/bcm963xx/shared/opensource/include/bcm963xx/bcmSpiRes.h */
		board_ioctl(BOARD_IOCTL_SPI_INIT, SPI_SLAVE_SELECT, 0, (char*)0, 0, 391000);
		board_ioctl_init();
	}
}
