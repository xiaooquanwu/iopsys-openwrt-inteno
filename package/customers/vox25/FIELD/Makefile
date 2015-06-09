#
# Copyright (C) 2013 Jo-Philipp Wich <jow@openwrt.org>
#
# Licensed under the Apache License, Version 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vox25-inteno-default
#PKG_VERSION:=$(shell git --git-dir=$(CURDIR)/../.git log -1 --pretty="%ci %h" | awk '{ print $$1 "-" $$4 }')
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/vox25-inteno-default
	
	CATEGORY:=Configs
  TITLE:=Inteno Field
  DEPENDS:=+vox25 +vodafone +rpcd +libubox +libubus
endef

define Package/vox25-inteno-default/config
	if PACKAGE_vox25-inteno-default-selected 
		config TARGET_CUSTOMER
			string "Customer ID"
			default "INTFIELD"
	endif
endef

define Package/vox25-inteno-default/description
	Vodafone VOX25 Inteno Default package
endef

define Package/vox25-inteno-default/install
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
endef

define Package/vox25-inteno-default/postinst

endef

$(eval $(call BuildPackage,vox25-inteno-default))
