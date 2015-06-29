#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=natalie-dect-h
PKG_VERSION:=11.19
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
ifeq ($(CONFIG_BCM_OPEN),y)
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/natalie-dect-h
else
PKG_SOURCE_URL:=git@iopsys.inteno.se:natalie-dect-h
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=c2139d05e3d082f1dcf58ce3f19306cb76fd0873
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk



define Package/natalie-dect-h
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Dect stack headers
  URL:=
endef

define Package/natalie-dect-h/description
	Kernel dect driver headers
endef


define Package/natalie-dect-h/install
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/natalie-dect
	$(CP) -r $(PKG_BUILD_DIR)/SrcHeaders/* $(STAGING_DIR)/usr/include/natalie-dect/
endef




$(eval $(call BuildPackage,natalie-dect-h))
