#
# Copyright (C) 2014 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=voice-client
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/voice-client
  SUBMENU:=Telephony
  SECTION:=net
  CATEGORY:=Network
  TITLE:=voice-client
endef

define Package/voice-client/description
	voice-client
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/voice-client/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,voice-client))
