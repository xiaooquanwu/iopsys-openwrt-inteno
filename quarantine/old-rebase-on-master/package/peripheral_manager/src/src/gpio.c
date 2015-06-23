#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "gpio.h"
#include "log.h"

static int brcmboard = -1;

void gpio_open_ioctl( void ) {

	if (brcmboard == -1){
		brcmboard = open("/dev/brcmboard", O_RDWR);
		if ( brcmboard == -1 ) {
			DBG(1,"failed to open: /dev/brcmboard\n");
			return;
		}
		DBG(1, "fd %d allocated\n", brcmboard);
	}

	return;
}

int board_ioctl(int ioctl_id, int action, int hex, char* string_buf, int string_buf_len, int offset) {
	BOARD_IOCTL_PARMS IoctlParms = {0};

	IoctlParms.string = string_buf;
	IoctlParms.strLen = string_buf_len;
	IoctlParms.offset = offset;
	IoctlParms.action = action;
	IoctlParms.buf    = (char*)"";

	if ( ioctl(brcmboard, ioctl_id, &IoctlParms) < 0 ) {
		syslog(LOG_INFO, "ioctl: %d failed", ioctl_id);
		exit(1);
	}
	return IoctlParms.result;
}
