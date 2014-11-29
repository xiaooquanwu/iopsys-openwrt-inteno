#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "smbus.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "ucix.h"
#include "i2c.h"
#include "log.h"

void dump_i2c(int fd,int start,int stop)
{
    int i;
    int res;

    for (i=start ; i < stop; i++) {
        res = i2c_smbus_read_byte_data(fd,i);
        if (res < 0){perror("i2c error\n");}
        DEBUG_PRINT("/dev/i2c-0 READ %d = 0x%02x\n",i,(unsigned char)res);
    }
}

int i2c_open_dev (const char *bus, int addr, unsigned long needed)
{
    int fd = open(bus, O_RDWR);
    if (fd < 0) {
        syslog(LOG_INFO,"%s: could not open /dev/i2c-0\n",__func__);
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        syslog(LOG_INFO,"%s: could not set address %x for i2c chip\n",
               __func__, addr);
    error:
        close (fd);
        return -1;
    }
    if (needed) {
        unsigned long funcs;
        if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
            syslog(LOG_INFO,"%s: could not get I2C_FUNCS\n",__func__);
            goto error;
        }
        if ( (funcs & needed) != needed) {
            syslog(LOG_INFO,"%s: lacking I2C capabilities, have %lx, need %lx\n",
                   __func__, funcs, needed);
            goto error;
        }
    }
    return fd;
}

void do_init_tab( struct i2c_touch *i2c_touch)
{
    const struct i2c_reg_tab *tab;
    int i;

    tab = i2c_touch->init_tab;

    for (i = 0 ; i < i2c_touch->init_tab_len ; i++){
        int y;
        int ret;
        for ( y = 0 ; y <= tab[i].range; y++ ){
//          DEBUG_PRINT("%s: addr %02X = %02X \n",__func__,(unsigned char)tab[i].addr+y, (unsigned char)tab[i].value);
            ret = i2c_smbus_write_byte_data(i2c_touch->dev, tab[i].addr+y, tab[i].value);
            if (ret < 0){
                perror("write to i2c dev\n");
            }
        }
    }
//  dump_i2c(i2c_touch->dev,0,13);

}


struct i2c_touch * i2c_init(struct uci_context *uci_ctx, char* i2c_dev_name, struct i2c_touch* i2c_touch_list, int len)
{
    const char *p;
    int i;

    p = ucix_get_option(uci_ctx, "hw", "board", "hardware");
    if (p == 0){
        syslog(LOG_INFO, "%s: Missing Hardware identifier in configuration. I2C is not started\n",__func__);
        return 0;
    }

    /* Here we match the hardware name to a init table, and get the
       i2c chip address */
    i2c_touch = NULL;
    for (i = 0; i < len; i++)
        if (!strcmp(i2c_touch_list[i].name, p)) {
            DEBUG_PRINT("I2C hardware platform %s found.\n", p);
            i2c_touch = &i2c_touch_list[i];
            break;
        }
    if (!i2c_touch) {
        DEBUG_PRINT("No I2C hardware found: %s.\n", p);
        return 0;
    }

    i2c_touch->dev = i2c_open_dev(i2c_dev_name, i2c_touch->addr,
                                  I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE);

    if (i2c_touch->dev < 0) {
        syslog(LOG_INFO,"%s: could not open i2c touch device\n",__func__);
        i2c_touch->dev = 0;
        return 0;
    }

    DEBUG_PRINT("Opened device and selected address %x \n", i2c_touch->addr);

    do_init_tab(i2c_touch);

    return i2c_touch;
}
