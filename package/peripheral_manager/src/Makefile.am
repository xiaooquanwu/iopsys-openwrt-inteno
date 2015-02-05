#PRG_VERSION := $(shell cd @top_srcdir@; if ! git describe --tags ;then echo version-$(PACKAGE_VERSION) ;fi )

bin_PROGRAMS = peripheral_manager
peripheral_manager_SOURCES = src/peripheral_manager.c

peripheral_manager_CFLAGS = -DPRG_VERSION=\"$(PRG_VERSION)\"
