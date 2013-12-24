include $(TOPDIR)/rules.mk

# We use the foldername as main package name
PKG_NAME:=ice-client

CFMODULES:=$(filter CONFIG_PACKAGE_$(PKG_NAME)-%,$(.VARIABLES))
CFMODULES:=$(subst CONFIG_PACKAGE_$(PKG_NAME)-,,$(CFMODULES))
$(info Building package: $(PKG_NAME))
$(info Building with additional modules: $(CFMODULES))
export CFMODULES

export HOME_DIR:=$(TOPDIR)

export PLATFORM_INCLUDE:=platforms/iopsys/build.mk

DATE:=$(shell date +%Y-%m-%d-%H-%M-%S)
TMP=$(shell echo $(DATE) > core/date.txt)
LOGIN:=$(shell whoami)
TMP=$(shell echo $(LOGIN) > core/login.txt)
#BASE_PKG_VERSION:=$(shell cat core/version.txt)
BASE_PKG_VERSION:=2.8.1
PKG_VERSION:=$(BASE_PKG_VERSION)_$(LOGIN)_$(DATE)
PKG_RELEASE:=RC1
PKG_SOURCE_URL:=ssh://git@iopsys.inteno.se/ice-client.git
PKG_SOURCE_VERSION:=02990460d3032aff5ced7a0ed64abd3c171e900a
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(BASE_PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(BASE_PKG_VERSION).tar.gz

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(BASE_PKG_VERSION)

RSTRIP:=true

include $(INCLUDE_DIR)/package.mk

define Package/ice-client/Default
	CATEGORY:=Utilities
	URL:=http://www.cloudfriends.eu/
	DEPENDS:= ice-client
	DEFAULT:=y
endef

define Package/ice-client
$(call Package/ice-client/Default)
	TITLE:=ice-client
	MENU:=1
	DEPENDS:=+libopenssl +libstdcpp
	DEFAULT:=n
endef

# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) config
	echo $(BASE_PKG_VERSION) > $(PKG_BUILD_DIR)/core/version.txt
endef

define Package/ice-client/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "backup current config"
    cp /etc/cloudfriends/server.ini /tmp/cloudfriendsserver.ini
fi
exit 0
endef

define Package/ice-client/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/core/ice $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/{libcli.so.1.0.1,systemService.so.1.0.1,libconfStore.so.1,libjsoncpp.so.1,networkService.so.1.0.1,packageService.so.1.0.1} $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/scripts/etc/config/cloudfriends $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/scripts/server.ini $(1)/etc/cloudfriends	
	$(CP) $(PKG_BUILD_DIR)/scripts/modify.ini $(1)/etc/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/scripts/*.crt $(1)/etc/cloudfriends
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/scripts/cloudfriends $(1)/etc/init.d/cloudfriends
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/admin
	$(CP) $(PKG_BUILD_DIR)/scripts/luci/controller/admin/*.lua $(1)/usr/lib/lua/luci/controller/admin
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/scripts/luci/model/cbi/cloudfriends/*.lua $(1)/usr/lib/lua/luci/model/cbi/cloudfriends
endef

define Package/ice-client/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Enabling rc.d symlink for cloudfriends"
    #/etc/init.d/cloudfriends enable
    /etc/init.d/cloudfriends stop
    cp /tmp/cloudfriendsserver.ini /etc/cloudfriends/server.ini 
    rm -f /tmp/cloudfriendsserver.ini 
    ice -a /etc/cloudfriends/modify.ini -c /etc/cloudfriends/server.ini
    if [ -f "/etc/rc.d/S99cloudfriends" ]; then
        /etc/init.d/cloudfriends start
    fi
fi
exit 0
endef

# Additional modules

# Camerawatch
define Package/ice-client-camerawatch
$(call Package/ice-client/Default)
	TITLE:=Camerawatch
	DEPENDS:=+libstdcpp +ice-client
endef

define Package/ice-client-camerawatch/description
	Camerawatch module for ice-client
endef

define Package/ice-client-camerawatch/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=cameramanager" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-camerawatch/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/core/lib/camerawatch.so.1.0.1 $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/scripts/reg_camera $(1)/etc/cloudfriends
endef

define Package/ice-client-camerawatch/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	grep "dhcp-script=" /etc/dnsmasq.conf || echo dhcp-script=/etc/cloudfriends/reg_camera >> /etc/dnsmasq.conf
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=camerawatch.so.1.0.1" > /tmp/cfin
fi
exit 0
endef

# Guest
define Package/ice-client-guest
$(call Package/ice-client/Default)
	TITLE:=Guest
	DEPENDS:=+libstdcpp +ice-client
endef

define Package/ice-client-guest/description
	Guest module for ice-client
endef

define Package/ice-client-guest/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=guestMgr" > /tmp/cfin
	sleep 5
fi
exit 0
endef

define Package/ice-client-guest/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/guestService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-guest/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=guestService.so.1.0.1" > /tmp/cfin
fi
exit 0
endef

# OpenVPN
define Package/ice-client-openvpn
$(call Package/ice-client/Default)
	TITLE:=OpenVPN
	DEPENDS:=+libstdcpp +ice-client
endef

define Package/ice-client-openvpn/description
	OpenVPN module for ice-client
endef

define Package/ice-client-openvpn/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=ovpnMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-openvpn/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/ovpnService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-openvpn/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=ovpnService.so.1.0.1" > /tmp/cfin
fi
exit 0
endef

# Anyfi
define Package/ice-client-anyfi
$(call Package/ice-client/Default)
	TITLE:=AnyFi
	DEPENDS:=+libstdcpp +ice-client
endef

define Package/ice-client-anyfi/description
	AnyFi module for ice-client
endef

define Package/ice-client-anyfi/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=anyfiMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-anyfi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/anyfiService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-anyfi/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=anyfiService.so.1.0.1" > /tmp/cfin
fi
exit 0
endef

# Logreader
define Package/ice-client-logreader
$(call Package/ice-client/Default)
	TITLE:=LogReader
	DEPENDS:=+libstdcpp +ice-client
endef

define Package/ice-client-logreader/description
	Logread module for ice-client
endef

define Package/ice-client-logreader/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=log" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-logreader/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/logService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-logreader/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=logService.so.1.0.1" > /tmp/cfin
fi
exit 0
endef

# FileMe
define Package/ice-client-fileme
$(call Package/ice-client/Default)
	TITLE:=FileMe
	DEPENDS:=+libstdcpp +lighttpd +samba2-client +ice-client
endef

define Package/ice-client-fileme/description
	FileMe module for ice-client
endef

define Package/ice-client-fileme/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleRemove topic=fileMeMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-fileme/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/filemeService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-fileme/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	head -n 1 /tmp/cfout &
	echo "system moduleAdd file=filemeService.so.1.0.1" > /tmp/cfin
fi
exit 0
endef



# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,ice-client))
$(eval $(call BuildPackage,ice-client-camerawatch))
$(eval $(call BuildPackage,ice-client-guest))
$(eval $(call BuildPackage,ice-client-openvpn))
$(eval $(call BuildPackage,ice-client-anyfi))
$(eval $(call BuildPackage,ice-client-logreader))
$(eval $(call BuildPackage,ice-client-fileme))
