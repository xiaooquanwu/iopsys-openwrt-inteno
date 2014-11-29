#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bcmhotproxy
PKG_RELEASE:=13

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

define Package/bcmhotproxy
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Daemon That feeds broadcom driver calls to hotplug2
endef

define Package/bcmhotproxy/description
 This package contains a Daemon that will listen to link events from broadcoms adsl driver and etherent driver and proxy them to hotplug
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

target=$(firstword $(subst -, ,$(BOARD)))

MAKE_FLAGS += TARGET="$(target)"
TARGET_CFLAGS += -Dtarget_$(target)=1 -Wall


define Package/bcmhotproxy/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d/

	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/etc/init.d/* $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bcmhotproxy $(1)/sbin/
endef

$(eval $(call BuildPackage,bcmhotproxy))
