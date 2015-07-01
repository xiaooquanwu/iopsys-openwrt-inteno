#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=endptcfg-open
PKG_VERSION:=15-06-24
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ihgsp.inteno.se/ihgsp/consumer/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/endptcfg-open
  CATEGORY:=Utilities
  TITLE:=Broadcom endpoint driver configuration application pre compiled for open SDK
  URL:=
endef

define Package/endptcfg-open/description
        Broadcom endpoint driver configuration application pre compiled for open SDK
endef



define Package/endptcfg-open/install

	ifeq ($(CONFIG_BCM_CHIP_ID),"63138")	
	$(CP) $(PKG_BUILD_DIR)/endptcfg_arm $(1)/usr/bin/endptcfg
	else
	$(CP) $(PKG_BUILD_DIR)/endptcfg_mips $(1)/usr/bin/endptcfg
	endif	 	

endef

$(eval $(call BuildPackage,endptcfg-open))
