#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_RELEASE:=1
PKG_VERSION:=1.1.0
ifeq ($(CONFIG_PACKAGE_bcmkernel),y)
PKG_SOURCE_URL:=git@iopsys.inteno.se:dectmngr2.git
else
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/dectmngr2.git
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=a51f8ed1b9178262777f73f7c275d3962f829c8a
PKG_NAME:=dectmngr2


LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true
export BUILD_DIR

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/dectmngr2
  CATEGORY:=Utilities
  TITLE:=Dectmngr2
  URL:=
endef

define Package/dectmngr2/description
	Dectmngr2
endef


define Package/dectmngr2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/dect/

	cp $(PKG_BUILD_DIR)/src/dectmngr2 $(1)/usr/bin/
	cp $(PKG_BUILD_DIR)/files/target.bin $(1)/etc/dect/
endef

$(eval $(call BuildPackage,dectmngr2))
