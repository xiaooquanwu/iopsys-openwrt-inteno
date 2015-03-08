# 
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bootchart2
PKG_VERSION:=0.14.7
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/mmeeks/bootchart.git
PKG_SOURCE_VERSION:=3ab81137cafe25c2ca4bc3a5f322a63646f9ce8d

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/bootchart2
 SECTION:=base
 CATEGORY:=Utilities
 TITLE:=Bootchart2
 URL:=https://github.com/mmeeks/bootchart
endef

define Package/bootchart2/description
 Bootchart is a tool for performance analysis and visualization of the GNU/Linux boot process. 
endef

define Package/bootchart2/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bootchart-collector $(1)/sbin
	$(CP) ./files/* $(1)/
endef




$(eval $(call BuildPackage,bootchart2))
