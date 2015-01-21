#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

export BUILD_DIR

PKG_NAME:=brcm_fw_tool
PKG_RELEASE:=1

PKG_BUILD_DIR := $(KERNEL_BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

define Package/brcm_fw_tool
  SECTION:=utils
  CATEGORY:=Base system
  TITLE:=Update utility for Broadcom nand firmware images
  DEPENDS:=+bcmkernel
endef

define Package/brcm_fw_tool/description
 This package contains an utility useful to upgrade from other firmware or 
 older OpenWrt releases.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

target=$(firstword $(subst -, ,$(BOARD)))

MAKE_FLAGS += TARGET="$(target)"
TARGET_CFLAGS += -Dtarget_$(target)=1 -Wall

ifdef CONFIG_MTD_REDBOOT_PARTS
  MAKE_FLAGS += FIS_SUPPORT=1
  TARGET_CFLAGS += -DFIS_SUPPORT=1
endif

define Package/brcm_fw_tool/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brcm_fw_tool $(1)/sbin/
endef

$(eval $(call BuildPackage,brcm_fw_tool))
