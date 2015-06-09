include $(TOPDIR)/rules.mk

PKG_NAME:=vox25
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

define Package/vox25
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=VOX25 Router
  DEPENDS:=+juci
endef

define Package/vox25/config
	if PACKAGE_vox25
		choice 
			prompt "Customer Config"
			default PACKAGE_vox25-vodafone
			config PACKAGE_vox25-vodafone
				bool "Vodafone"
			config PACKAGE_vox25-inteno-default
				bool "Inteno Default"
		endchoice
	endif
endef

define Package/vox25/description
	Vodafone VOX25 base package
endef

define Package/vox25/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/htdocs/* $(1)/www/
	#$(1)/sbin/juci-update $(1)/www/ RELEASE
endef

define Package/vox25/postinst
#!/bin/sh
/sbin/juci-update /www/ RELEASE
exit 0
endef

$(eval $(call BuildPackage,vox25))
