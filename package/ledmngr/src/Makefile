# Makefile for ledmngr

CC = gcc
CFLAGS += -Wall
CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx/
CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/
OBJS = ucix.o brcmdaemon.o ledmngr.o smbus.o

ledmngr: $(OBJS)
	$(CC) $(LDFLAGS) -o ledmngr $(OBJS) -luci -lubus -lubox -lblobmsg_json

clean:
	rm -f *.o

