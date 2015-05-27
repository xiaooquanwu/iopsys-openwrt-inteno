# 
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_VERSION:=1.5.1
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_SOURCE_URL:=git@iopsys.inteno.se:mtd-utils
PKG_SOURCE_PROTO:=git

PKG_SOURCE_VERSION:=1e2156fa3d8272566f6c62e7be1fe3d9de9a5ebc
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

FILES:= \
	docfdisk \
	doc_loadbios \
	flashcp \
	flash_erase \
	flash_lock \
	flash_otp_dump \
	flash_otp_info \
	flash_otp_lock \
	flash_otp_write \
	flash_unlock \
	ftl_check \
	ftl_format \
	imagewrite \
	jffs2dump \
	jffs2reader \
	mkfs.jffs2 \
	mtd_debug \
	nanddump \
	nandtest \
	nandwrite \
	nftldump \
	nftl_format \
	recv_image \
	rfddump \
	rfdformat \
	serve_image \
	sumtool

ifneq ($(CONFIG_LINUX_2_4),y)
FILES += \
	mkfs.ubifs \
	mtdinfo \
	ubiattach \
	ubiblock \
	ubicrc32 \
	ubidetach \
	ubiformat \
	ubimkvol \
	ubinfo \
	ubinize \
	deubinize \
	ubirename \
	ubirmvol \
	ubirsvol \
	ubiupdatevol
endif

define PartGen
define Package/mtd-utils-$(subst _,-,$(1))
  TITLE:=MTD $(1)
  URL:=http://www.linux-mtd.infradead.org/
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=mtd-utils +zlib +liblzo +libuuid
endef
endef

define Package/mtd-utils
  TITLE:=Utilities for flash info/debug
  SECTION:=utils
  CATEGORY:=Utilities
  MENU:=1
endef

define Package/mtd-utils/description
  Utilities for manipulating memory technology devices.
endef

define Package/mtd-utils/install
	true
endef

$(foreach file,$(FILES),$(eval $(call PartGen,$(file))))

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	BUILDDIR="$(PKG_BUILD_DIR)" \
	LDLIBS+="$(LIBGCC_S)" \
	WITHOUT_XATTR=1

ifeq ($(CONFIG_LINUX_2_4),y)
MAKE_FLAGS += \
	SUBDIRS=""
endif

define PartInstall
define Package/mtd-utils-$(subst _,-,$(1))/install
	$(INSTALL_DIR) \
		$$(1)/usr/sbin

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/$(1) \
		$$(1)/usr/sbin/
endef
endef

$(foreach file,$(FILES),$(eval $(call PartInstall,$(file))))

$(eval $(call BuildPackage,mtd-utils))
$(foreach file,$(FILES),$(eval $(call BuildPackage,mtd-utils-$(subst _,-,$(file)))))
