
include $(TOPDIR)/rules.mk

PKG_NAME:=juci
PKG_VERSION:=15.6.2
#PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git@iopsys.inteno.se:juci.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE_VERSION:=HEAD
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MAINTAINER:=Martin Schröder <mkschreder.uk@gmail.com>

PKG_LICENSE:=GPL
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

define Build/Configure
	$(shell "cd $(PKG_BUILD_DIR) && git pull")
endef
#define Build/Configure
	#(cd $(PKG_BUILD_DIR) && $(BASH) -x ./bootstrap)
#	$(CP) ./share ./htdocs $(PKG_BUILD_DIR)/
#	$(call Build/Configure/Default)
#endef


define Package/juci
  $(Package/juci/default)
  SECTION:=juci
  CATEGORY:=JUCI
  TITLE:=JUCI Web Based Interface
  DEPENDS:=+rpcd +luciexpress +uhttpd +uhttpd-mod-ubus +libubox +libubus
endef

define Package/juci/description
 Provides the JUCI Javascript UCI web interface with standard functionality.
endef

define Package/juci/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/htdocs/* $(1)/www/
	$(INSTALL_DIR) $(1)/usr/share/ubus-plugind/
	$(CP) $(PKG_BUILD_DIR)/menu.d $(1)/usr/share/ubus-plugind/
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/juci.config $(1)/etc/config/juci
	$(INSTALL_DIR) $(1)/www/cgi-bin/
	$(CP) $(PKG_BUILD_DIR)/juci_redirect.sh $(1)/www/cgi-bin/juci
	$(INSTALL_DIR) $(1)/sbin/
	$(CP) $(PKG_BUILD_DIR)/juci-update $(1)/sbin/juci-update
endef

define Package/juci/postinst
#!/bin/sh

if [ "$$(uci -q get uhttpd.main.ubus_prefix)" != "/ubus" ]; then
	uci set uhttpd.main.ubus_prefix="/ubus"
	uci commit uhttpd
fi

exit 0
endef

$(eval $(call BuildPackage,juci))
