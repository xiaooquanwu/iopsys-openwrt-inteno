# 
# Copyright (C) 2012 Aleksander Morgado <aleksander@lanedo.com>
# Copyright (C) 2013 Rusink <rzrusink@qmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmbim
PKG_VERSION:=1.8.0
PKG_RELEASE:=1

PKG_SOURCE:=libmbim-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://cgit.freedesktop.org/libmbim/libmbim/snapshot/

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libmbim
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +zlib
  TITLE:=libmbim
  URL:=http://www.lanedo.com/~aleksander
endef

define Package/libmbim/description
  Helper library and utils to talk to MBIM enabled modems
endef

define Build/Configure
	$(SH) $(PKG_BUILD_DIR)/autogen.sh
	$(Build/Configure/Default)
endef

CONFIGURE_VARS+= CFLAGS="$$$$CFLAGS -DMESSAGE_ENABLE_TRACE"

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libmbim-glib \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} \
		$(1)/usr/lib/
endef

define Package/libmbim/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/

endef

$(eval $(call BuildPackage,libmbim))
