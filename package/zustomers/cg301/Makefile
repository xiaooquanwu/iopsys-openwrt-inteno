include $(TOPDIR)/rules.mk

PKG_NAME:=cg301
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

define Package/cg301
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=cg301 Router
  DEPENDS:=+juci +CONFIG_TARGET_ROOTFS_UBIFS
endef

define Package/cg301/config
	if PACKAGE_cg301
		choice 
			prompt "Customer Config"
			default PACKAGE_cg301-vodafone
			config PACKAGE_cg301-vodafone
				bool "Vodafone"
			config PACKAGE_cg301-inteno-default
				bool "Inteno Default"
		endchoice
	endif
endef

define Package/cg301/description
	Vodafone cg301 base package
endef

define Package/cg301/install
	
endef

define Package/cg301/postinst

endef

$(eval $(call BuildPackage,cg301))
