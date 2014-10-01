#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_RELEASE:=1
PKG_VERSION:=1.0.1
ifeq ($(CONFIG_PACKAGE_bcmkernel),y)
PKG_SOURCE_URL:=git@iopsys.inteno.se:teliasafety-misc.git
else
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/teliasafety-misc.git
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=8feb071dc9c05b5580043718aaafe424ce557afe
PKG_NAME:=teliasafety-misc


LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/teliasafety-misc
  CATEGORY:=Utilities
  TITLE:=Misc stuff for teliasafety
  URL:=
  DEPENDS:=
endef

define Package/teliasafety-misc/description
	Broadcom endpoint test application
endef


define Package/teliasafety-misc/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	cp $(PKG_BUILD_DIR)/files/etc/init.d/* $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,teliasafety-misc))
