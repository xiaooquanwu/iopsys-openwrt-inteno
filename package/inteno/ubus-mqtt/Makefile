#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ubus-mqtt
PKG_VERSION:=14.20.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=git@iopsys.inteno.se:$(PKG_NAME).git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

CONFIGURE_VARS += \
	BUILD_CC="$(HOSTCC)" \
	TARGET_CC="$(TARGET_CC)"

TARGET_CFLAGS += \
	-L$(STAGING_DIR)/usr/lib -I$(STAGING_DIR)/usr/include

define Package/ubus-mqtt
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ubus-mqtt - a ubus replicator for mqtt
  URL:=http://www.alleato.se/
  MAINTAINER:=Martin Assarsson
  DEPENDS:=+jansson +libpthread +libmosquitto +libopenssl +ubus +libubox
endef

define Package/ubus-mqtt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/ubus-mqtt $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/ubus-mqtt.init $(1)/etc/init.d/ubus-mqtt
endef

define Package/ubus-mqtt/postinst
#!/bin/sh
#/etc/init.d/ubus-mqtt enable
#/etc/init.d/ubus-mqtt restart
endef

$(eval $(call BuildPackage,ubus-mqtt))


