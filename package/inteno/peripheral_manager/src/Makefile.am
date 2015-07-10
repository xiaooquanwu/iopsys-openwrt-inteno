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
	src/sim_led.c \
	src/button.h \
	src/button.c \
	src/sim_button.c \
	src/catv.c \
	src/catv_monitor.c \
	src/catv.h \
	src/smbus.c \
	src/smbus.h \
	src/i2c.c \
	src/i2c.h \
	src/sfp.c \
	src/sfp.h

if BRCM_BOARD

bin_PROGRAMS += vox_test gpio_test

peripheral_manager_SOURCES += \
	src/gpio_led.c \
	src/gpio_led.h \
	src/gpio_button.c \
	src/gpio_button.h \
	src/gpio.c \
	src/gpio.h \
	src/gpio_shift_register.c \
	src/gpio_shift_register.h \
	src/touch_sx9512.c \
	src/touch_sx9512.h \
	src/sx9512.c \
	src/sx9512.h \
	src/vox.h \
	src/vox.c \
	src/prox_px3220.c

vox_test_SOURCES = \
	src/vox_test.c \
	src/gpio.c \
	src/gpio.h

gpio_test_SOURCES = \
	src/gpio_test.c \
	src/gpio.c \
	src/gpio.h \
	src/gpio_shift_register.c \
	src/gpio_shift_register.h \
	src/smbus.c \
	src/smbus.h \
	src/i2c.c \
	src/i2c.h \
	src/sx9512.c \
	src/sx9512.h

endif

peripheral_manager_LDADD = $(UCI_LIB) $(UBOX_LIB) $(UBUS_LIB) -lm

peripheral_manager_CFLAGS =  $(AM_CFLAGS) -DPRG_VERSION=\"$(PRG_VERSION)\" 
