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
	int            addr;
	led_state_t    state;
	struct led_drv led;
};

void vox_init(struct server_ctx *s_ctx);

static int vox_set_state(struct led_drv *drv, led_state_t state)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
        char spi_data[6] = {0,0,0,0,0,0};

        if (p->state == state)
                    return state;

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

        DBG(1,"vox_set_state %x %x %x %x",spi_data[0],spi_data[1],spi_data[2],spi_data[3]);
	board_ioctl(BOARD_IOCTL_SPI_WRITE, SPI_SLAVE_SELECT, 0, spi_data, 6, 0);

	p->state = state;
        return state;
}

static led_state_t vox_get_state(struct led_drv *drv)
{
	struct vox_data *p = (struct vox_data *)drv->priv;
	return p->state;
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
	.set_state = vox_set_state,
	.get_state = vox_get_state,
	.support = vox_support,
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
		led_add(&data->led);
		register_spi = 1;
	}

	/* if config entries for vox leds exist register the spi as used. */
	if(register_spi) {
		board_ioctl(BOARD_IOCTL_SPI_INIT, SPI_SLAVE_SELECT, 0, 0, 0, 391000);
		gpio_open_ioctl();
	}
}
