#PRG_VERSION := $(shell cd @top_srcdir@; if ! git describe --tags ;then echo version-$(PACKAGE_VERSION) ;fi )

AM_CFLAGS = $(OUR_CFLAGS)

bin_PROGRAMS = peripheral_manager

peripheral_manager_SOURCES = \
	src/peripheral_manager.c \
	src/ucix.c \
	src/ucix.h

peripheral_manager_LDADD = $(UCI_LIB) $(UBOX_LIB) $(UBUS_LIB)

peripheral_manager_CFLAGS =  $(AM_CFLAGS) -DPRG_VERSION=\"$(PRG_VERSION)\"
