include $(TOPDIR)/rules.mk

PKG_NAME:=crashlog
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/crashlog
  CATEGORY:=Utilities
  TITLE:=Handling kernel crash logs
endef

define Package/crashlog/description
	Handling kernel crash logs.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/crashlog/install
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,crashlog))
