#ifndef I2C_H
#define I2C_H

void dump_i2c(int fd,int start,int stop);
int i2c_open_dev (const char *bus, int addr, unsigned long needed);

#endif
