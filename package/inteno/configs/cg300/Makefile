include $(TOPDIR)/rules.mk

PKG_NAME:=routermodel-cg300
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./juci-theme-vodafone/* $(PKG_BUILD_DIR)/
endef

define Package/routermodel-cg300
	CATEGORY:=Configs
  TITLE:=CG300 Router
  DEPENDS:=+juci +juci-theme-default
  HIDDEN:=1
endef

define Package/routermodel-cg300/config

endef

define Package/routermodel-cg300/description
	Inteno CG300 specific box and theming package. 
endef

define Package/routermodel-cg300/install
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
endef

define Package/routermodel-cg300/postinst

endef

$(eval $(call BuildPackage,routermodel-cg300))
