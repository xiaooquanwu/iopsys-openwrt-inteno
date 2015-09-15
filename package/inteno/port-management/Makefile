include $(TOPDIR)/rules.mk

PKG_NAME:=port-management
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/port-management
  CATEGORY:=Base system
  TITLE:=Port management tool
endef

define Package/port-management/description
	port-management contains necessary tools to create
	layer2 interfaces
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/port-management/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,port-management))
