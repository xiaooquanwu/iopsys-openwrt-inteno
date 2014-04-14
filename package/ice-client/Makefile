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

export DATE:=$(shell date +%Y-%m-%d-%H-%M-%S)
export LOGIN:=$(shell whoami)
BASE_PKG_VERSION:=2.9.5
PKG_RELEASE:=RC5
PKG_VERSION:=$(BASE_PKG_VERSION)-$(PKG_RELEASE)_$(DATE)_$(LOGIN)

###########################--RELEASE--################################

PKG_SOURCE_URL:=ssh://git@iopsys.inteno.se/ice-client.git
PKG_SOURCE_VERSION:=ec157f81dad22aec407c2fd75efab9a90290ec53
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(BASE_PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(BASE_PKG_VERSION)-$(PKG_RELEASE).tar.gz

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(BASE_PKG_VERSION)


###########################--RELEASE--################################

include $(INCLUDE_DIR)/package.mk

define Package/ice-client/Default
	CATEGORY:=Utilities
	URL:=http://inteno.se/
	DEPENDS:= ice-client
endef

define Package/ice-client
$(call Package/ice-client/Default)
	TITLE:=ice-client
	MENU:=1
	DEPENDS:=+libopenssl +libstdcpp
	DEFAULT:=y
endef

###########################--DEVELOPMENT--################################
# Development settings
# Uncomment stuff  Build/Prepare
#
# Plus make sure to comment and provide the following lines above
# #PKG_SOURCE_URL:=ssh://git@iopsys.inteno.se/ice-client.git
# #PKG_SOURCE_VERSION:=a11bf64e6b1054803b52750aabd71bf440ff6e16
# #PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(BASE_PKG_VERSION)
# #PKG_SOURCE:=$(PKG_NAME)-$(BASE_PKG_VERSION)-$(PKG_RELEASE).tar.gz
# 
# PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(BASE_PKG_VERSION)
# 
#
# These settings allow you to specify a soruce folder where your source code is located
# This avoids having to tar-zip or fetch from git each time
# Changes can be done in your sources while the build is performed somewhere else
# If you forget some settings, your source folder will be deleted!!! So be carefull and 
# check in or backup your sources first before doing this.
#
#define Build/Prepare
#	rm -rf $(PKG_BUILD_DIR)
#	ln -s /home/erik/Desktop/cloned_inteno_cfmod/iopsys-client $(PKG_BUILD_DIR)
#endef

###########################--DEVELOPMENT--################################


#
# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.

define Build/Configure
	$(MAKE) -C $(PKG_BUILD_DIR) config
	echo $(BASE_PKG_VERSION)-$(PKG_RELEASE) > $(PKG_BUILD_DIR)/core/version.txt
	echo $(DATE) > $(PKG_BUILD_DIR)/core/date.txt
	echo $(LOGIN) > $(PKG_BUILD_DIR)/core/login.txt
endef

define Build/Compile	
	$(MAKE) -C $(PKG_BUILD_DIR) 
endef

define Package/ice-client/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "backup current config"
    cp /etc/cloudfriends/server.ini /tmp/cloudfriendsserver.ini
    cp /etc/config/ice /tmp/ice.back
fi
exit 0
endef

define Package/ice-client/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/core/ice $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/{libcli.so.1.0.1,systemService.so.1.0.1,libconfStore.so.1,libjsoncpp.so.1,networkService.so.1.0.1,packageService.so.1.0.1} $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/scripts/etc/config/ice $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/scripts/server.ini $(1)/etc/cloudfriends	
	$(CP) $(PKG_BUILD_DIR)/scripts/modify.ini $(1)/etc/cloudfriends
	$(CP) $(PKG_BUILD_DIR)/scripts/*.crt $(1)/etc/cloudfriends
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/scripts/ice-client $(1)/etc/init.d/ice-client
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(CP) $(PKG_BUILD_DIR)/scripts/etc/uci-defaults/luci-ice-client $(1)/etc/uci-defaults/luci-ice-client
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller/
	$(CP) $(PKG_BUILD_DIR)/scripts/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/ice
	$(CP) $(PKG_BUILD_DIR)/scripts/luci/model/cbi/ice/*.lua $(1)/usr/lib/lua/luci/model/cbi/ice
endef

define Package/ice-client/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo "Enabling rc.d symlink for cloudfriends"
    cp /tmp/cloudfriendsserver.ini /etc/cloudfriends/server.ini 
    rm -f /tmp/cloudfriendsserver.ini 
    cp /tmp/ice.back /etc/config/ice 
    rm -f /tmp/ice.back
    ice -a /etc/cloudfriends/modify.ini -c /etc/cloudfriends/server.ini
    if [ -f "/etc/rc.d/S99ice-client" ]; then
        /etc/init.d/ice-client start
    fi
fi
exit 0
endef

# Additional modules

# Camerawatch
define Package/ice-client-camerawatch
$(call Package/ice-client/Default)
TITLE:=Camerawatch
DEPENDS+= +libjpeg 
endef

define Package/ice-client-camerawatch/description
Camerawatch module for ice-client
endef

define Package/ice-client-camerawatch/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  echo "system moduleRemove topic=cameramanager" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-camerawatch/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  echo "system moduleRemove topic=cameramanager" > /tmp/cfin
	  /bin/sleep 3
	fi
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
    grep "dhcp-script=" /etc/dnsmasq.conf || /bin/echo dhcp-script=/etc/cloudfriends/reg_camera >> /etc/dnsmasq.conf
    read -t 1 <>/tmp/cfout
	if [ -f /tmp/ice.pid  ]; then
		/bin/echo "system moduleAdd file=camerawatch.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef

# Guest
define Package/ice-client-guest
  $(call Package/ice-client/Default)
  TITLE:=Guest
  DEPENDS+= 
endef

define Package/ice-client-guest/description
Guest module for ice-client
endef

define Package/ice-client-guest/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=guestMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-guest/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=guestMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
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
    read -t 1 <>/tmp/cfout
	if [ -f /tmp/ice.pid  ]; then
		/bin/echo "system moduleAdd file=guestService.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef

# OpenVPN
define Package/ice-client-openvpn
  $(call Package/ice-client/Default)
  TITLE:=OpenVPN
  DEPENDS+= +openvpn
endef

define Package/ice-client-openvpn/description
OpenVPN module for ice-client
endef

define Package/ice-client-openvpn/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=ovpnMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-openvpn/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=ovpnMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
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
    read -t 1 <>/tmp/cfout
	if [ -f /tmp/ice.pid  ]; then
		/bin/echo "system moduleAdd file=ovpnService.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef

# Anyfi
define Package/ice-client-anyfi
  $(call Package/ice-client/Default)
  TITLE:=AnyFi
  DEPENDS+= 
endef

define Package/ice-client-anyfi/description
AnyFi module for ice-client
endef

define Package/ice-client-anyfi/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=anyfiMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

# Netrounds
define Package/ice-client-netrounds
  $(call Package/ice-client/Default)
  TITLE:=Netrounds
  DEPENDS+= 
endef

define Package/ice-client-netrounds/description
Netrounds module for ice-client
endef

define Package/ice-client-netrounds/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=netroundsMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-netrounds/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=netroundsMgr" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-netrounds/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/netroundsService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-netrounds/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    read -t 1 <>/tmp/cfout
	if [ -f /tmp/ice.pid  ]; then
		/bin/echo "system moduleAdd file=netroundsService.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef

# Logreader
define Package/ice-client-logreader
  $(call Package/ice-client/Default)
  TITLE:=LogReader
  DEPENDS+=
endef

define Package/ice-client-logreader/description
Logread module for ice-client
endef

define Package/ice-client-logreader/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=log" > /tmp/cfin
	  /bin/sleep 3
	fi
fi
exit 0
endef

define Package/ice-client-logreader/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  /bin/echo "system moduleRemove topic=log" > /tmp/cfin
	  /bin/sleep 3
	fi
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
    read -t 1 <>/tmp/cfout
	if [ -f /tmp/ice.pid  ]; then
		echo "system moduleAdd file=logService.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef


# fileme
define Package/ice-client-fileme
  $(call Package/ice-client/Default)
  TITLE:=FileMe
  DEPENDS+= +lighttpd +samba2-client
endef

define Package/ice-client-fileme/description
FileMe module for ice-client
endef

define Package/ice-client-fileme/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  echo "system moduleRemove topic=fileMeMgr" > /tmp/cfin
	  /bin/sleep 2
	fi
fi
exit 0
endef

define Package/ice-client-fileme/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  echo "system moduleRemove topic=fileMeMgr" > /tmp/cfin
	  /bin/sleep 2
	fi
fi
exit 0
endef

define Package/ice-client-fileme/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/filemeService.so.1.0.1 $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/cloudfriends/fileme/lighttpd/conf.d
	$(CP) $(PKG_BUILD_DIR)/scripts/fileme/mime.conf $(1)/etc/cloudfriends/fileme/lighttpd
	$(CP) $(PKG_BUILD_DIR)/scripts/fileme/conf.d/10-webdav.conf $(1)/etc/cloudfriends/fileme/lighttpd/conf.d
	$(INSTALL_DIR) $(1)/etc/cloudfriends/fileme/docroot/dav
	$(INSTALL_DIR) $(1)/etc/cloudfriends/fileme/docroot/shares
endef

define Package/ice-client-fileme/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	PROC_ID=$$(cat /tmp/ice.pid)
	PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
	if [  $$PROC_EXISTS ]; then
	  read -t 1 <>/tmp/cfout
	  echo "system moduleAdd file=filemeService.so.1.0.1" > /tmp/cfin
	fi
fi
exit 0
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,ice-client))
$(eval $(call BuildPackage,ice-client-guest))
$(eval $(call BuildPackage,ice-client-camerawatch))
$(eval $(call BuildPackage,ice-client-openvpn))
$(eval $(call BuildPackage,ice-client-anyfi))
$(eval $(call BuildPackage,ice-client-netrounds))
$(eval $(call BuildPackage,ice-client-logreader))
$(eval $(call BuildPackage,ice-client-fileme))
