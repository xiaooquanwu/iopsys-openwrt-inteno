#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=brcminfo
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

define Package/brcminfo
  CATEGORY:=Utilities
  TITLE:=Broadcom voice driver info utility
  URL:=
  DEPENDS:= +bcmkernel
endef

define Package/brcminfo/description
	Utility that prints information regarding voice configuration
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

target=$(firstword $(subst -, ,$(BOARD)))

MAKE_FLAGS += TARGET="$(target)"


EXTRA_CFLAGS += -DBOS_OS_LINUXUSER

EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx/
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc/
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc/
EXTRA_CFLAGS +=	-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc

TARGET_CFLAGS += -D$(CONFIG_TARGET_IBOARDID) -Dtarget_$(target)=1 -Wall

define Package/brcminfo/install
	$(INSTALL_DIR) $(1)/usr/bin
	cp $(PKG_BUILD_DIR)/brcminfo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,brcminfo))
