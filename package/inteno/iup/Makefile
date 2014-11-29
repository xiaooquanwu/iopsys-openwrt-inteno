#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iup
PKG_VERSION:=0.0.1
PKG_RELEASE:=0

include $(INCLUDE_DIR)/package.mk

define Package/iup
  SECTION:=net
  CATEGORY:=Network
  TITLE:=iup client
  DEPENDS:=+libuci  +uci +busybox
endef

define Package/iup/description
	This package contains Intenos IUP Client
endef

define Build/Compile
	true
endef

define Package/iup/install
	$(INSTALL_DIR) $(1)/sbin
	cp -r ./files/*  $(1)/
endef

$(eval $(call BuildPackage,iup))
