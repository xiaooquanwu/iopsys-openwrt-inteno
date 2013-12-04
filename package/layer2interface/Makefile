#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=layer2interface
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/layer2interface
  CATEGORY:=Base system
  TITLE:=Tools for setting up layer2 interfaces
endef

define Package/layer2interface/description
	layer2interface contains necessary tools to create
	layer2 interfaces
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./broadcom/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/layer2interface/install
	$(CP) ./broadcom/* $(1)/
endef

$(eval $(call BuildPackage,layer2interface))
