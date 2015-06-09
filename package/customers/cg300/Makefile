include $(TOPDIR)/rules.mk

PKG_NAME:=cg300
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

define Package/cg300
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=cg300 Router
  DEPENDS:=+juci +CONFIG_TARGET_ROOTFS_UBIFS
endef

define Package/cg300/config
	if PACKAGE_cg300
		choice 
		prompt "Customer Config"
		default PACKAGE_cg300_field
		config PACKAGE_cg300_anyfi
			bool "Anyfi"
		config PACKAGE_cg300_field
			bool "Inteno Default"
		config PACKAGE_cg300_blc
			bool "Blc"
		config PACKAGE_cg300_console
			bool "Console"
		config PACKAGE_cg300_elements
			bool "Elements"
		config PACKAGE_cg300_get
			bool "Get"
		config PACKAGE_cg300_lounea
			bool "Lounea"
		config PACKAGE_cg300_netmedia
			bool "Netmedia"
		config PACKAGE_cg300_nordix
			bool "Nordix"
		config PACKAGE_cg300_teliasafety
			bool "Telia Safety"
	endchoice
	endif
endef

define Package/cg300/description
	Vodafone cg300 base package
endef

define Package/cg300/install
	
endef

define Package/cg300/postinst

endef

$(eval $(call BuildPackage,cg300))
