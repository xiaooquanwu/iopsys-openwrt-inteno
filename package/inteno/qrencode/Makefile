#
# Copyright (C) 2009 ePoint Systems Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=qrencode
PKG_VERSION:=3.0.3
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib -lpng

TARGET_CFLAGS += $(FPIC)

define Package/qrencode
  CATEGORY:=Utilities
  DEPENDS:=+libpng
  TITLE:=QRcode encoder library
  URL:=http://megaui.net/fukuchi/works/qrencode/index.en.html
endef

define Package/qrencode/description
	Libqrencode is a C library for encodingdata in a QR Code symbol, a
	kind of 2D symbology that can be scanned by handy terminals such as
	a mobile phone with CCD. The capacity of QR Code is up to 7000
	digits or 4000 characters, and is highly robust.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include"
endef

define Package/qrencode/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/qrencode $(1)/usr/bin/
endef

$(eval $(call BuildPackage,qrencode))
