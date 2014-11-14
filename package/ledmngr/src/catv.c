#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "i2c.h"
#include "catv.h"

struct catv_handler
{
    int i2c_fd;
};

struct catv_handler * catv_init(char *i2c_bus,int i2c_addr)
{
    struct catv_handler *h;

    printf("%s:\n",__func__);

    h = malloc( sizeof(struct catv_handler) );

    if (!h)
        return NULL;

    h->i2c_fd = i2c_open_dev(i2c_bus, i2c_addr,
                             I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);

    if (h->i2c_fd == -1 ){
        syslog(LOG_INFO,"Did not find any CATV device at %s address %x \n", i2c_bus, i2c_addr);
        free(h);
        return 0;
    }

    dump_i2c(h->i2c_fd,0,255);

    return h;
}

void catv_destroy(struct catv_handler *h)
{
    free(h);
}
