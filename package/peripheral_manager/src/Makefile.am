#PRG_VERSION := $(shell cd @top_srcdir@; if ! git describe --tags ;then echo version-$(PACKAGE_VERSION) ;fi )

AM_CFLAGS = $(OUR_CFLAGS)

bin_PROGRAMS = peripheral_manager
dist_data_DATA = configs/hw

peripheral_manager_SOURCES = \
	src/peripheral_manager.c \
	src/ucix.c \
	src/ucix.h \
	src/server.c \
	src/server.h \
	src/led.c \
	src/led.h \
	src/sim_led.c

peripheral_manager_LDADD = $(UCI_LIB) $(UBOX_LIB) $(UBUS_LIB)

peripheral_manager_CFLAGS =  $(AM_CFLAGS) -DPRG_VERSION=\"$(PRG_VERSION)\"
