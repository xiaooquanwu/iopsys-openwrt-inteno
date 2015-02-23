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

#include "i2c.h"
#include "log.h"

void dump_i2c(int fd,int start,int stop)
{
	int i;
	int res;

	for (i=start ; i < stop; i++) {
		res = i2c_smbus_read_byte_data(fd,i);
		if (res < 0){perror("i2c error\n");}
		DBG(1,"/dev/i2c-0 READ %d = 0x%02x\n",i,(unsigned char)res);
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

