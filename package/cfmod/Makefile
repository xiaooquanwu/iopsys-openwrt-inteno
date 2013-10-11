#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

export PLATFORM_INCLUDE := platforms/iopsys/build.mk
export HOME_DIR:=$(TOPDIR)

PKG_RELEASE:=RC1
PKG_VERSION:=1.0.3

PKG_SOURCE_URL:=ssh://inteno@ihgsp.inteno.se/home/inteno/private/git/cfmod.git
PKG_SOURCE_VERSION:=4c8bcfc558b2a7b8f645626fad8e5d287f98dc00
PKG_NAME:=cfmod

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

RSTRIP:=true
export BUILD_DIR

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/cfmod
  CATEGORY:=Utilities
  TITLE:=Cloud friends client
  URL:=
  DEPENDS:= +libstdcpp +libopenssl
endef

define Package/cfmod/description
	Cloud friends client
endef

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) config
endef

define Package/cfmod/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "backup current config"
    cp /etc/cloudfriends/server.ini /tmp/cloudfriendsserver.ini
fi
exit 0
endef

define Package/cfmod/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/core/cf $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/core/lib/* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/etc/config/cloudfriends $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/cloudfriends
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/server.ini $(1)/etc/cloudfriends   
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/modify.ini $(1)/etc/cloudfriends   
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/*.crt $(1)/etc/cloudfriends
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/cloudfriends $(1)/etc/init.d/cloudfriends
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/luci/controller/admin/*.lua $(1)/usr/lib/lua/luci/controller/admin
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/cloudfriends
	$(CP) $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)/scripts/luci/model/cbi/cloudfriends/*.lua $(1)/usr/lib/lua/luci/model/cbi/cloudfriends
endef


define Package/cfmod/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Enabling rc.d symlink for cloudfriends"
    /etc/init.d/cloudfriends stop
    cp /tmp/cloudfriendsserver.ini /etc/cloudfriends/server.ini 
    rm -f /tmp/cloudfriendsserver.ini 
    cf -a /etc/cloudfriends/modify.ini -c /etc/cloudfriends/server.ini
    if [ -f "/etc/rc.d/S99cloudfriends" ]; then 
        /etc/init.d/cloudfriends start 
    fi
fi
exit 0
endef

$(eval $(call BuildPackage,cfmod))
