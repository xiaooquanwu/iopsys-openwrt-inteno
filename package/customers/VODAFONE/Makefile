#
# Copyright (C) 2013 Jo-Philipp Wich <jow@openwrt.org>
#
# Licensed under the Apache License, Version 2.0.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vodafone
#PKG_VERSION:=$(shell git --git-dir=$(CURDIR)/../.git log -1 --pretty="%ci %h" | awk '{ print $$1 "-" $$4 }')
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/vodafone
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=Vodafone Customer Package
endef


define Package/vodafone/config
	
endef

define Package/vodafone/description
	Vodafone specific base package
endef

define Package/vodafone/install
	
endef

define Package/vodafone/postinst

endef

$(eval $(call BuildPackage,vodafone))
