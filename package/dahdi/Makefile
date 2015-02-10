#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dahdi
PKG_VERSION:=2.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git@iopsys.inteno.se:dahdi
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk



BCM_KERNEL_VERSION=`cat $(BUILD_DIR)/bcmkernel/bcm963xx/kernel/linux-*/include/config/kernel.release`

export BUILD_DIR
export BCM_KERNEL_VERSION


define Package/dahdi
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Kernel timer for Asterisk meetme
  URL:=
  DEPENDS:= +bcmkernel
endef

define Package/dahdi/description
	Kernel timer for Asterisk meetme
endef


define Package/dahdi/install

# install header files
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/dahdi
	$(CP) -r $(PKG_BUILD_DIR)/linux/include/dahdi/* $(STAGING_DIR)/usr/include/dahdi

	mkdir -p $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/
	$(CP) $(PKG_BUILD_DIR)/linux/drivers/dahdi/dahdi.ko $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/

	mkdir -p $(1)/etc/modules.d/
	$(CP) etc/modules.d/90-dahdi $(1)/etc/modules.d/
endef

$(eval $(call BuildPackage,dahdi))
