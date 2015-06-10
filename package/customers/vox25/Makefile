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
  TITLE:=VOX25 Router Base Package
endef

define Package/vox25/config
	if PACKAGE_inteno-vox25
		choice 
			prompt "VOX25 Customer Config"
			default PACKAGE_inteno-vox25-vodafone
			config PACKAGE_inteno-vox25-vodafone
				bool "VOX25 Vodafone"
			config PACKAGE_inteno-vox25-default
				bool "VOX25 Default"
		endchoice
	endif
endef

define Package/vox25/description
	VOX25 base package
endef

define Package/vox25/install
	
endef

define Package/vox25/postinst

endef

$(eval $(call BuildPackage,vox25))
