#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=voicesec
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

define Package/voicesec
  CATEGORY:=Utilities
  TITLE:=VoIP security application
  URL:=
  DEPENDS:=
endef

define Package/voicesec/description
	VoIP security application
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

target=$(firstword $(subst -, ,$(BOARD)))

MAKE_FLAGS += TARGET="$(target)"

TARGET_CFLAGS += -D$(CONFIG_TARGET_IBOARDID) -Dtarget_$(target)=1 -Wall

define Package/voicesec/install
	$(INSTALL_DIR) $(1)/usr/bin
	cp $(PKG_BUILD_DIR)/voicesec $(1)/usr/bin/
endef

$(eval $(call BuildPackage,voicesec))
