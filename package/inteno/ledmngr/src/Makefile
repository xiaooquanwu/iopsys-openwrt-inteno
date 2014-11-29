# Makefile for ledmngr

CC = gcc
CFLAGS += -Wall -g
CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx/
CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/
OBJS = ucix.o brcmdaemon.o ledmngr.o smbus.o catv.o i2c.o sfp.o touch_sx9512.o led.o button.o spi.o

ledmngr: $(OBJS)
	$(CC) $(LDFLAGS) -o ledmngr $(OBJS) -luci -lubus -lubox -lblobmsg_json -lm

clean:
	rm -f *.o

