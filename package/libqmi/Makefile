include $(TOPDIR)/rules.mk

PKG_NAME:=libqmi
PKG_VERSION:=1.10.0
PKG_RELEASE:=1

PKG_SOURCE:=libqmi-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://cgit.freedesktop.org/libqmi/snapshot/

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/libqmi
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +zlib
  TITLE:=Helper library and utils to talk to QMI enabled modems
endef

define Package/libqmi/description
  Helper library and utils to talk to QMI enabled modems
endef

define Build/Configure
	$(SH) $(PKG_BUILD_DIR)/autogen.sh
	$(Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/libqmi-glib \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.{so*,a,la} \
		$(1)/usr/lib/
endef

define Package/libqmi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qmi-proxy \
		$(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libqmi))
