#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_RELEASE:=1
PKG_VERSION:=1
PKG_SOURCE_URL:=git://git.carnivore.it/users/common/af_alg.git
PKG_SOURCE_VERSION:=1851bbb010c38878c83729be844f168192059189

PKG_NAME:=af_alg
PKG_VERSION:=0.1

DEPENDS:=libopenssl

export BUILD_DIR

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz


TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib
MAKE_FLAGS += TARGET="$(target)"
TARGET_CFLAGS += -Dtarget_$(target)=1 -Wall

include $(INCLUDE_DIR)/package.mk


define Package/af_alg
  CATEGORY:=Libraries
  TITLE:=Openssl af_alg engine
  URL:=
endef

define Package/af_alg/description
	Openssl af_alg engine
endef

define Build/Compile
	$(MAKE_VARS) \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
	$(MAKE_FLAGS)
endef

define Package/af_alg/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/engines
	cp $(PKG_BUILD_DIR)/libaf_alg.so $(1)/usr/lib/engines/
endef

$(eval $(call BuildPackage,af_alg))
