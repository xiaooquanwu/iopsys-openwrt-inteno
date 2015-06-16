#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include <board.h>

#include "ucix.h"
#include "log.h"
#include "led.h"
#include "ledmngr.h"

struct spi {
    unsigned int slave_select;
    const char *name;
} *spi_dev;

struct spi spi_list[] = {
    {.name = "VOX25",
     .slave_select = 1,
    },
};

int spi_init(struct uci_context *uci_ctx) {
	const char *p;
    int i;

    p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
    if (p == 0){
        syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. SPI is not inited\n",__func__);
        return 0;
    }
	DEBUG_PRINT("%s %p %s\n", p, spi_list, spi_list[0].name);

    /* Here we match the hardware name to a init table, and get the
       i2c chip address */
    spi_dev = NULL;
    for (i = 0; i < sizeof(spi_list) / sizeof(spi_list[0]); i++) {
		DEBUG_PRINT("%s %s\n", spi_list[i].name, p);
        if (!strcmp(spi_list[i].name, p)) {
            spi_dev = &spi_list[i];
            DEBUG_PRINT("SPI hardware platform %s found with slave select id %d.\n", p, spi_dev->slave_select);
            break;
        }
    }
    if (!spi_dev) {
        DEBUG_PRINT("No SPI hardware platform found: %s.\n", p);
		return -1;
	}

    board_ioctl(BOARD_IOCTL_SPI_INIT, spi_dev->slave_select, 0, 0, 0, 391000);
	return 0;
}

void spi_led_set(struct led_config* lc, int state) {
	char buf[6] = {0,0,0,0,0,0};

    DEBUG_PRINT("Led %s: %d\n",lc->name, state);

	buf[0] = lc->address;
    if (state == ON)
        buf[1] = 1;
    else if (state == OFF)
        buf[1] = 0;
    else{
        DEBUG_PRINT("Led %s: Set to not supported state %d\n",lc->name, state);
        return;
    }

	board_ioctl(BOARD_IOCTL_SPI_WRITE, spi_dev->slave_select, 0, buf, 6, 0);
}

