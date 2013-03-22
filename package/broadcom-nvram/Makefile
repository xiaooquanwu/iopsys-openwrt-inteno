#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=broadcom-nvram
PKG_RELEASE:=1

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true

define Package/bcmnvram
  CATEGORY:=Libraries
  TITLE:=Broadcom nvram emulator library
  URL:=
  DEPENDS:=PACKAGE_libuci:libuci
endef

define Package/bcmnvram/description
	Broadcom nvram to uci wrapper
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

target=$(firstword $(subst -, ,$(BOARD)))

MAKE_FLAGS += TARGET="$(target)"
TARGET_CFLAGS += -Dtarget_$(target)=1 -Wall

define Package/bcmnvram/install
	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	cp $(PKG_BUILD_DIR)/libnvram.so $(1)/usr/lib/
	cp $(PKG_BUILD_DIR)/uci_test $(1)/usr/bin/
endef

$(eval $(call BuildPackage,bcmnvram))
