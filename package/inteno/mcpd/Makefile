#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mcpd
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/mcpd
  CATEGORY:=Network
  SUBMENU:=IPTV
  TITLE:=Multicast Daemon
endef

define Package/mcpd/description
	Multicast Daemon
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/mcpd/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,mcpd))
