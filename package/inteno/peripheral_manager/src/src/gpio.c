#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "gpio.h"
#include "log.h"

#define DEVFILE_BRCM_BOARD "/dev/brcmboard"

static int brcmboard = -1;


int board_ioctl_init(void) {
	if (brcmboard == -1){
		brcmboard = open(DEVFILE_BRCM_BOARD, O_RDWR);
		if ( brcmboard == -1 ) {
			syslog(LOG_ERR, "failed to open: %s", DEVFILE_BRCM_BOARD);
			return 1;
		}
		DBG(1, "fd %d allocated\n", brcmboard);
	}
	return 0;
}


int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
	BOARD_IOCTL_PARMS IoctlParms = {0};

	IoctlParms.string = string_buf;
	IoctlParms.strLen = string_buf_len;
	IoctlParms.offset = offset;
	IoctlParms.action = action;
	IoctlParms.buf    = (char*)"";

	if ( ioctl(brcmboard, ioctl_id, &IoctlParms) < 0 ) {
		syslog(LOG_ERR, "ioctl: %d failed", ioctl_id);
		return(-255);
	}
	return IoctlParms.result;
}


gpio_state_t gpio_get_state(gpio_t gpio)
{
	return board_ioctl(BOARD_IOCTL_GET_GPIO, 0, 0, NULL, gpio, 0);
}


void gpio_set_state(gpio_t gpio, gpio_state_t state)
{
	board_ioctl(BOARD_IOCTL_SET_GPIO, 0, 0, NULL, gpio, state);
}
