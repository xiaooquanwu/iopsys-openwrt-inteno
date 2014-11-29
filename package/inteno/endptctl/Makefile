#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_RELEASE:=1
PKG_VERSION:=benjamin
PKG_SOURCE_URL:=ssh://inteno@ihgsp.inteno.se/home/inteno/private/git/$(PKG_VERSION)/endptctl.git

PKG_NAME:=endptctl
PKG_VERSION:=0.1

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true
export BUILD_DIR

BCMKERNEL_DIR:=$(BUILD_DIR)/bcmkernel-3.4-4.14/bcm963xx
export BCMKERNEL_DIR

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/endptctl
  CATEGORY:=Utilities
  TITLE:=Broadcom endpoint test application
  URL:=
endef

define Package/endptctl/description
	Broadcom endpoint test application
endef


define Package/endptctl/install
	$(INSTALL_DIR) $(1)/usr/bin
	cp $(PKG_BUILD_DIR)/endptctl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,endptctl))
