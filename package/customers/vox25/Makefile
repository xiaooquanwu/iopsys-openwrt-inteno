#
# Copyright (C) 2013 Jo-Philipp Wich <jow@openwrt.org>
#
# Licensed under the Apache License, Version 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vox25
#PKG_VERSION:=$(shell git --git-dir=$(CURDIR)/../.git log -1 --pretty="%ci %h" | awk '{ print $$1 "-" $$4 }')
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/vox25
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=VOX25 Router
  DEPENDS:=@PACKAGE_vox25_selected +vodafone +CONFIG_TARGET_ROOTFS_UBIFS
endef

define Package/vox25/config
	
	if PACKAGE_vox25_selected 
		choice 
			prompt "Customer"
			default PACKAGE_vox25-vodafone-selected
			config PACKAGE_vox25-vodafone-selected
				select PACKAGE_vox25-vodafone
				bool "Vodafone"
			config PACKAGE_vox25-inteno-default-selected
				select PACKAGE_vox25-inteno-default
				bool "Inteno Default"
		endchoice
	endif
endef

define Package/vox25/description
	Vodafone VOX25 base package
endef

define Package/vox25/install
	
endef

define Package/vox25/postinst

endef

$(eval $(call BuildPackage,vox25))
