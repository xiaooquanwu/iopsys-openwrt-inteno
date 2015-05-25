#
# Copyright (C) 2015 Inteno
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifimngr
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

define Package/wifimngr
  CATEGORY:=Network
  DEPENDS:=+libpthread +libstdcpp +librt +libjson +alljoyn +ubus +libuci
  TITLE:=WiFi Manager
endef

define Package/wifimngr/description
	wifimngr manages wireless driver
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/wifimngr/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifimngr $(1)/sbin/
endef

$(eval $(call BuildPackage,wifimngr))
