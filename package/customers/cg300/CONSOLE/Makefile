include $(TOPDIR)/rules.mk

PKG_NAME:=customer-console-cg300
PKG_VERSION:=1.0.0
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/customer-console-cg300
	HIDDEN:=1
  TITLE:=Inteno Field
  DEPENDS:=+cg300 +screen
endef

define Package/customer-console-cg300/config

endef

define Package/customer-console-cg300/description
	Inteno Default package
endef

define Package/customer-console-cg300/install
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
endef

define Package/customer-console-cg300/postinst

endef

#$(eval $(call BuildPackage,customer-console-cg300))
