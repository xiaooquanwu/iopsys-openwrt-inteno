#
# Copyright (C) 2013 Jo-Philipp Wich <jow@openwrt.org>
#
# Licensed under the Apache License, Version 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=customer-vodafone-vox25
#PKG_VERSION:=$(shell git --git-dir=$(CURDIR)/../.git log -1 --pretty="%ci %h" | awk '{ print $$1 "-" $$4 }')
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./fs/* $(PKG_BUILD_DIR)/
endef

define Package/customer-vodafone-vox25
	HIDDEN:=1
  TITLE:=VODAFONE VOX25 Customer Configuration
  DEPENDS:=+routermodel-vox25 +juci
endef

define Package/customer-vodafone-vox25/description
	Vodafone VOX25 base package
endef

define Package/customer-vodafone-vox25/install
	$(CP) ./fs/etc $(1)/
endef

define Package/customer-vodafone-vox25/postinst

endef

$(eval $(call BuildPackage,customer-vodafone-vox25))
