include $(TOPDIR)/rules.mk

PKG_NAME:=routermodel-vox25
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

define Package/routermodel-vox25
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=VOX25 Router
  DEPENDS:=+juci 
endef

define Package/routermodel-vox25/config
	
endef

define Package/routermodel-vox25/description
	Inteno VOX25 specific box and theming package. 
endef

define Package/routermodel-vox25/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/htdocs/* $(1)/www/
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
	#$(1)/sbin/juci-update $(1)/www/ RELEASE
endef


define Image/PreBuild
	CONFIG_TARGET_MARTINTEST:="MARTINTEST is SET"
endef

define Package/routermodel-vox25/postinst
#!/bin/sh
/sbin/juci-update /www/ RELEASE
exit 0
endef

$(eval $(call BuildPackage,routermodel-vox25))
