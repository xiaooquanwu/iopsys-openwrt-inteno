#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=brcm-h
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git@iopsys.inteno.se:brcm-h
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk



define Package/brcm-h
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Broadcom header files
  URL:=
endef

define Package/brcm-h/description
	Broadcom header files
endef


define Package/brcm--h/install
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/brcm
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/bcmdrivers/broadcom/include/bcm963xx/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/bcmdrivers/opensource/include/bcm963xx/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/voice_res_gw/inc/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/voice_res_gw/codec/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc/* $(STAGING_DIR)/usr/include/brcm \
	$(CP) -r $(PACKAGE_BUILD_DIR)/bcm963xx/xChange/dslx_common/xchg_drivers/inc/* $(STAGING_DIR)/usr/include/brcm \

endef




$(eval $(call BuildPackage,brcm-h))
