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
BASE_PKG_VERSION:=3.4.0
PKG_RELEASE:=RC1
PKG_VERSION:=$(BASE_PKG_VERSION)-$(PKG_RELEASE)_$(DATE)_$(LOGIN)
export PKG_VERSION

###########################--RELEASE--################################

PKG_SOURCE_URL:=ssh://git@iopsys.inteno.se/ice-client.git
PKG_SOURCE_VERSION:=7380810fcbfb5f44c2eb073df48a28942c81056d
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
	DEPENDS:=+libopenssl +libstdcpp +libubus +ubus +ubusd +jshn +libubox
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
#	ln -s /home/erik/Desktop/ice-client $(PKG_BUILD_DIR)
##	ln -s /home/erik/Desktop/checkCompilation/iopsys-client $(PKG_BUILD_DIR)
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
	echo $(BASE_PKG_VERSION)-$(PKG_RELEASE) > $(PKG_BUILD_DIR)/core/version.txt
	echo $(DATE) > $(PKG_BUILD_DIR)/core/date.txt
	echo $(LOGIN) > $(PKG_BUILD_DIR)/core/login.txt
	$(MAKE) -C $(PKG_BUILD_DIR) config
endef

define Build/Clean
	$(MAKE) -C $(PKG_BUILD_DIR) clean
endef

define Build/Compile	
	$(MAKE) -C $(PKG_BUILD_DIR) config
	$(MAKE) -C $(PKG_BUILD_DIR) 
endef

define Package/ice-client/preinst
#!/bin/sh
echo "backup current config"
cp /etc/iopsys/server.ini /tmp/iopsysserver.ini
cp /etc/config/ice /tmp/ice.back
exit 0
endef

define Package/ice-client/install
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/core/ice $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/modules/system/ubusevent/obj/iopsys/ubusevent $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/{systemService.so.1.0.1,libconfStore.so.1,networkService.so.1.0.1,packageService.so.1.0.1,monitorService.so.1.0.1,iperfService.so.1.0.1,logService.so.1.0.1} $(1)/usr/lib
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/execute
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/progress
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/result
	$(CP) $(PKG_BUILD_DIR)/scripts/scheduleiperf.sh $(1)/bin/ifs/iperf/execute
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) $(PKG_BUILD_DIR)/scripts/etc/config/ice $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/iopsys
	$(CP) $(PKG_BUILD_DIR)/scripts/server.ini $(1)/etc/iopsys	
	$(CP) $(PKG_BUILD_DIR)/scripts/modify.ini $(1)/etc/iopsys
	$(INSTALL_DIR) $(1)/etc/iopsys/certificates
	$(CP) $(PKG_BUILD_DIR)/scripts/cert_server_ca.pem $(1)/etc/iopsys/certificates
	$(CP) $(PKG_BUILD_DIR)/scripts/inteno_root_ca.pem $(1)/etc/iopsys/certificates
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_BUILD_DIR)/scripts/ice-client $(1)/etc/init.d/ice-client
endef

define Package/ice-client/postinst
#!/bin/sh
echo "Enabling rc.d symlink for iopsys"
cp /tmp/iopsysserver.ini /etc/iopsys/server.ini 
rm -f /tmp/iopsysserver.ini 
cp /tmp/ice.back /etc/config/ice 
rm -f /tmp/ice.back
if [ -f "/etc/rc.d/S99ice-client" ]; then
	/etc/init.d/ice-client start
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=cameramanager" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-camerawatch/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=cameramanager" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-camerawatch/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/iopsys
	$(CP) $(PKG_BUILD_DIR)/core/lib/camerawatch.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-camerawatch/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	/bin/echo "system moduleAdd file=camerawatch.so.1.0.1" package="ice-client-camerawatch" > /tmp/cfin
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=guestMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-guest/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=guestMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-guest/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/guestService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-guest/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	/bin/echo "system moduleAdd file=guestService.so.1.0.1" package="ice-client-guest" > /tmp/cfin
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=ovpnMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-openvpn/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=ovpnMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-openvpn/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/ovpnService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-openvpn/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	/bin/echo "system moduleAdd file=ovpnService.so.1.0.1" package="ice-client-openvpn" > /tmp/cfin
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=anyfiMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-anyfi/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=anyfiMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-anyfi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/anyfiService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-anyfi/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	/bin/echo "system moduleAdd file=anyfiService.so.1.0.1" package="ice-client-anyfi" > /tmp/cfin
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=netroundsMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-netrounds/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=netroundsMgr" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-netrounds/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/netroundsService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-netrounds/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	/bin/echo "system moduleAdd file=netroundsService.so.1.0.1" package="ice-client-netrounds" > /tmp/cfin
fi
exit 0
endef

# Logreader
define Package/ice-client-logreader
  $(call Package/ice-client/Default)
  TITLE:=LogReader
  DEPENDS+= 
  # Actually logreader still works without all this
  # however when collectd support is needed, all this is required on top
  # +collectd +libltdl +libiptc +collectd-mod-network +collectd-mod-memory +collectd-mod-cpu +collectd-mod-interface
endef

define Package/ice-client-logreader/description
Logread module for ice-client
endef

define Package/ice-client-logreader/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=log" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-logreader/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  /bin/echo "system moduleRemove topic=log" > /tmp/cfin
  /bin/sleep 3
fi
exit 0
endef

define Package/ice-client-logreader/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/logService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-logreader/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=logService.so.1.0.1" package="ice-client-logreader" > /tmp/cfin
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
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=fileMeMgr" > /tmp/cfin
  /bin/sleep 2
fi
exit 0
endef

define Package/ice-client-fileme/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=fileMeMgr" > /tmp/cfin
  /bin/sleep 2
fi
exit 0
endef

define Package/ice-client-fileme/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/filemeService.so.1.0.1 $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/iopsys/fileme/lighttpd/conf.d
	$(CP) $(PKG_BUILD_DIR)/scripts/fileme/mime.conf $(1)/etc/iopsys/fileme/lighttpd
	$(CP) $(PKG_BUILD_DIR)/scripts/fileme/conf.d/10-webdav.conf $(1)/etc/iopsys/fileme/lighttpd/conf.d
	$(INSTALL_DIR) $(1)/etc/iopsys/fileme/docroot/dav
	$(INSTALL_DIR) $(1)/etc/iopsys/fileme/docroot/shares
endef

define Package/ice-client-fileme/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=filemeService.so.1.0.1" package="ice-client-fileme" > /tmp/cfin
fi
exit 0
endef

# example
define Package/ice-client-example
  $(call Package/ice-client/Default)
  TITLE:=example
  DEPENDS+= 
endef

define Package/ice-client-example/description
example module for ice-client
endef

define Package/ice-client-example/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=example" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-example/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=example" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-example/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/exampleService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-example/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=exampleService.so.1.0.1" package="ice-client-example" > /tmp/cfin
fi
exit 0
endef

# iperf
define Package/ice-client-iperf
  $(call Package/ice-client/Default)
  TITLE:=iperf
  DEPENDS+= 
endef

define Package/ice-client-iperf/description
iperf module for ice-client
endef

define Package/ice-client-iperf/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=iperfMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-iperf/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=iperfMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-iperf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/iperfService.so.1.0.1 $(1)/usr/lib
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/execute
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/progress
	$(INSTALL_DIR) $(1)/bin/ifs/iperf/result
	$(CP) $(PKG_BUILD_DIR)/scripts/scheduleiperf.sh $(1)/bin/ifs/iperf/execute
endef

define Package/ice-client-iperf/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=iperfService.so.1.0.1" package="ice-client-iperf" > /tmp/cfin
fi
exit 0
endef

# rshell
define Package/ice-client-rshell
  $(call Package/ice-client/Default)
  TITLE:=rshell
  DEPENDS+= 
endef

define Package/ice-client-rshell/description
rshell module for ice-client
endef

define Package/ice-client-rshell/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=rshellMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-rshell/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=rshellMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-rshell/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/rshellService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-rshell/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=rshellService.so.1.0.1" package="ice-client-rshell" > /tmp/cfin
fi
exit 0
endef

# rework
define Package/ice-client-rework
  $(call Package/ice-client/Default)
  TITLE:=rework
  DEPENDS+= 
endef

define Package/ice-client-rework/description
rework module for ice-client
endef

define Package/ice-client-rework/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=rework" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-rework/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=rework" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-rework/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/reworkService.so.1.0.1 $(1)/usr/lib
	$(INSTALL_DIR) $(1)/bin
	$(CP) $(PKG_BUILD_DIR)/modules/rework/sshpass-1.05/obj/iopsys/sshpass $(1)/bin
endef

define Package/ice-client-rework/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=reworkService.so.1.0.1" package="ice-client-rework" > /tmp/cfin
fi
exit 0
endef

# sip
define Package/ice-client-sip
  $(call Package/ice-client/Default)
  TITLE:=sip
  DEPENDS+= 
endef

define Package/ice-client-sip/description
sip module for ice-client
endef

define Package/ice-client-sip/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=sip" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-sip/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=sip" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-sip/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/sipService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-sip/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=sipService.so.1.0.1" package="ice-client-sip" > /tmp/cfin
fi
exit 0
endef

# monitor
define Package/ice-client-monitor
  $(call Package/ice-client/Default)
  TITLE:=monitor
  DEPENDS+= 
endef

define Package/ice-client-monitor/description
monitor module for ice-client
endef

define Package/ice-client-monitor/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=monitor" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-monitor/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=monitor" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-monitor/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/monitorService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-monitor/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=monitorService.so.1.0.1" package="ice-client-monitor" > /tmp/cfin
fi
exit 0
endef

# torrent
define Package/ice-client-torrent
  $(call Package/ice-client/Default)
  TITLE:=torrent
  DEPENDS+= +boost-system
#  boost                                   Boost C++ source libraries (header-only)
#  boost-chrono                            Boost C++ source libraries (chrono)
#  boost-date_time                         Boost C++ source libraries (date_time)
#  boost-filesystem                        Boost C++ source libraries (filesystem)
#  boost-graph                             Boost C++ source libraries (graph)
#  boost-iostreams                         Boost C++ source libraries (iostreams)
#  boost-locale                            Boost C++ source libraries (locale)
#  boost-math                              Boost C++ source libraries (math)
#  boost-program_options                   Boost C++ source libraries (program_options)
#  boost-python                            Boost C++ source libraries (python)
#  boost-random                            Boost C++ source libraries (random)
#  boost-regex                             Boost C++ source libraries (regex)
#  boost-serialization                     Boost C++ source libraries (serialization)
#  boost-signals                           Boost C++ source libraries (signals)
#  boost-timer                             Boost C++ source libraries (timer)
#boost-wave                              Boost C++ source libraries (wave)

endef

define Package/ice-client-torrent/description
torrent module for ice-client
endef

define Package/ice-client-torrent/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=torrentMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-torrent/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=torrentMgr" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-torrent/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/torrentService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-torrent/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=torrentService.so.1.0.1" package="ice-client-torrent" > /tmp/cfin
fi
exit 0
endef

# gigaset
define Package/ice-client-gigaset
  $(call Package/ice-client/Default)
  TITLE:=gigaset
  DEPENDS+= 
endef

define Package/ice-client-gigaset/description
gigaset module for ice-client
endef

define Package/ice-client-gigaset/prerm
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=gigaset" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-gigaset/preinst
#!/bin/sh
if [ ! -f "/tmp/ice.pid" ] ; then exit 0 ; fi
PROC_ID=$$(cat /tmp/ice.pid)
PROC_EXISTS=$$(/usr/bin/pgrep -P $${PROC_ID})
if [ -n "$$PROC_EXISTS" ]; then
  read -t 1 <>/tmp/cfout
  echo "system moduleRemove topic=gigaset" > /tmp/cfin
fi
exit 0
endef

define Package/ice-client-gigaset/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/core/lib/gigasetService.so.1.0.1 $(1)/usr/lib
endef

define Package/ice-client-gigaset/postinst
#!/bin/sh
read -t 1 <>/tmp/cfout
if [ -f /tmp/ice.pid  ]; then
	echo "system moduleAdd file=gigasetService.so.1.0.1" package="ice-client-gigaset" > /tmp/cfin
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
$(eval $(call BuildPackage,ice-client-example))
$(eval $(call BuildPackage,ice-client-iperf))
$(eval $(call BuildPackage,ice-client-rshell))
$(eval $(call BuildPackage,ice-client-rework))
$(eval $(call BuildPackage,ice-client-sip))
$(eval $(call BuildPackage,ice-client-monitor))
$(eval $(call BuildPackage,ice-client-torrent))
$(eval $(call BuildPackage,ice-client-gigaset))
