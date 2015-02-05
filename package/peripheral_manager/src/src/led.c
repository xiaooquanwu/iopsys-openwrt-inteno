#include <syslog.h>
#include "log.h"
#include "led.h"

void led_add( struct led_drv *drv)
{
	DBG(1,"called with led name [%s]\n", drv->name);
}

