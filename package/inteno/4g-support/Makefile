include $(TOPDIR)/rules.mk

PKG_NAME:=4g-support
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/4g-support
  CATEGORY:=Network
  TITLE:=4G Network Setup Tools
endef

define Package/4g-support/description
	4g-support contains necessary tools to setup 4G WAN connection
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/4g-support/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,4g-support))
