#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_RELEASE:=1
PKG_VERSION:=1.0.5
ifeq ($(CONFIG_PACKAGE_bcmkernel),y)
PKG_SOURCE_URL:=git@iopsys.inteno.se:dect-prog.git
else
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/dect-prog.git
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=1bf14349ff080e9ea294641b8297a436d1384c58
PKG_NAME:=dect-prog


LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true
export BUILD_DIR

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/dect-prog
  CATEGORY:=Utilities
  TITLE:=Dect programming tool
  URL:=
endef

define Package/dect-prog/description
	Dect programming tool
endef


define Package/dect-prog/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/dect/

	cp $(PKG_BUILD_DIR)/src/dect-prog $(1)/usr/bin/
	cp $(PKG_BUILD_DIR)/files/dect-reset $(1)/usr/bin/
	cp $(PKG_BUILD_DIR)/files/target.bin $(1)/etc/dect/
endef

$(eval $(call BuildPackage,dect-prog))
