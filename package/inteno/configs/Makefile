include $(TOPDIR)/rules.mk

PKG_NAME:=inteno-configs
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/inteno-configs
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=Configs Base Package
endef

define Package/inteno-configs/config
	choice
		prompt "Select Inteno Target Hardware"
		default PACKAGE_inteno-vox25
		config PACKAGE_inteno-vox25
			select TARGET_ROOTFS_UBIFS
			bool "VOX25"
		config PACKAGE_inteno-cg300
			bool "CG300"
		config PACKAGE_inteno-cg301
			bool "CG301"
	endchoice
endef

define Package/inteno-configs/description
	Placeholder package for inteno base configs
endef

define Package/inteno-configs/install
	
endef

define Package/inteno-configs/postinst

endef

$(eval $(call BuildPackage,inteno-configs))
