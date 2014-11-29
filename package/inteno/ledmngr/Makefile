#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ledmngr
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

export BUILD_DIR

include $(INCLUDE_DIR)/package.mk

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
	-Wl,-rpath-link=$(STAGING_DIR)/lib


define Package/ledmngr
  CATEGORY:=Utilities
  TITLE:=Application deamon for handling of leds
  URL:=
  DEPENDS:=+libuci +libubus +libblobmsg-json +bcmkernel
endef

define Package/ledmngr/description
	Application deamon for handling of leds
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef


define Package/ledmngr/install
#	$(CP) ./files/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d/

	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/etc/init.d/* $(1)/etc/init.d/
	cp $(PKG_BUILD_DIR)/ledmngr $(1)/sbin/
endef

$(eval $(call BuildPackage,ledmngr))
