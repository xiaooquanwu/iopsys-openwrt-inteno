include $(TOPDIR)/rules.mk

PKG_NAME:=targets
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/targets
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=Configs Base Package
endef

define Package/targets/config
	choice
		prompt "Selected Box Target"
		default PACKAGE_vox25
		config PACKAGE_vox25
			select TARGET_ROOTFS_UBIFS
			bool "VOX25"
		config PACKAGE_cg300
			bool "CG300"
		config PACKAGE_cg301
			bool "CG301"
	endchoice
endef

define Package/targets/description
	Board-specific config package
endef

define Package/targets/install
	
endef

define Package/targets/postinst

endef

$(eval $(call BuildPackage,targets))
