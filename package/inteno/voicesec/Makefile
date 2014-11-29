#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=voicesec
PKG_VERSION:=1.0

PKG_SOURCE_URL:=git@ihgsp.inteno.se:voicesec
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=661c0091e579e284712aeed892e921cc84a6f0bb
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

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

#target=$(firstword $(subst -, ,$(BOARD)))
#MAKE_FLAGS += TARGET="$(target)"
#TARGET_CFLAGS += -D$(CONFIG_TARGET_IBOARDID) -Dtarget_$(target)=1 -Wall

define Package/voicesec/install
	$(INSTALL_DIR) $(1)/usr/bin
	cp $(PKG_BUILD_DIR)/voicesec $(1)/usr/bin/
endef

$(eval $(call BuildPackage,voicesec))
