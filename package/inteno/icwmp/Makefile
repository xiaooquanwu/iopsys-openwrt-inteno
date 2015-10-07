#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=icwmp
PKG_VERSION:=3.0-2015-10-07
PKG_FIXUP:=autoreconf
ifeq ($(CONFIG_BCM_OPEN),y)
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/freecwmp.git
else
PKG_SOURCE_URL:=ssh://git@iopsys.inteno.se/freecwmp.git
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=77bffe53a98653c405d49e69ba2ed602c4f5090b
PKG_RELEASE=$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)


PKG_CONFIG_DEPENDS:= \
	CONFIG_CWMP_ACS_MULTI \
	CONFIG_CWMP_ACS_HDM \
	CONFIG_CWMP_DEBUG \
	CONFIG_CWMP_DEVEL_DEBUG

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

CWMP_REVISION=$(shell svnversion ./src/ -n|cut -f2 -d:)

include $(INCLUDE_DIR)/package.mk

define Package/icwmp/Default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CWMP client
  DEPENDS:=+libuci +libmicroxml +libubox +libjson +jshn +libubus +libblobmsg-json +ubusd +shflags +getopt
endef

define Package/icwmp/description
 A free client implementation of CWMP (TR-069) protocol
endef

define Package/icwmp-curl
  $(call Package/icwmp/Default)
  TITLE+= (using libcurl)
  DEPENDS+= +libcurl
  VARIANT:=curl
endef

define Package/icwmp-zstream
  $(call Package/icwmp/Default)
  TITLE+= (using libzstream)
  DEPENDS+= +libzstream
  VARIANT:=zstream
endef

define Package/icwmp-zstream/config
	source "$(SOURCE)/Config.in"
endef

USE_LOCAL=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)
ifneq ($(USE_LOCAL),)
define Build/Prepare
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef
endif

TARGET_CFLAGS += \
	-D_GNU_SOURCE -D_AADJ
	
ifneq ($(CWMP_REVISION)_,_)
ifneq ($(CWMP_REVISION),exported)
ifneq ($(CWMP_REVISION),Unversioned directory)
TARGET_CFLAGS += "-DCWMP_REVISION=\\\"$(CWMP_REVISION)\\\""
TARGET_LDFLAGS += "-DCWMP_REVISION=\\\"$(CWMP_REVISION)\\\""
endif
endif
endif

TARGET_LDFLAGS += \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

CONFIGURE_ARGS += \
	--with-uci-include-path=$(STAGING_DIR)/usr/include \
	--with-libubox-include-path=$(STAGING_DIR)/usr/include \
	--with-libubus-include-path=$(STAGING_DIR)/usr/include

ifeq ($(BUILD_VARIANT),zstream)
CONFIGURE_ARGS += \
	--enable-http=zstream \
	--with-zstream-include-path=$(STAGING_DIR)/usr/include
endif

ifeq ($(BUILD_VARIANT),curl)
CONFIGURE_ARGS += \
	--enable-http=curl
endif

ifeq ($(CONFIG_CWMP_ACS_MULTI),y)
CONFIGURE_ARGS += \
	--enable-acs=multi
endif

ifeq ($(CONFIG_CWMP_ACS_HDM),y)
CONFIGURE_ARGS += \
	--enable-acs=hdm
endif

ifeq ($(CONFIG_CWMP_DEBUG),y)
CONFIGURE_ARGS += \
	--enable-debug
endif

ifeq ($(CONFIG_CWMP_DEVEL_DEBUG),y)
CONFIGURE_ARGS += \
	--enable-devel
endif

define Package/icwmp-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) $(1)/etc/icwmpd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/bin/icwmpd $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/config/icwmp $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/config/idmmap $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/config/firewall.icwmp $(1)/etc/firewall.icwmp
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init/icwmpd.init $(1)/etc/init.d/icwmpd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uci-defaults/90-icwmpfirewall $(1)/etc/uci-defaults/90-icwmpfirewall
ifeq ($(CONFIG_CWMP_SCRIPTS_FULL),y)
	$(INSTALL_DIR) $(1)/usr/share/icwmp
	$(CP) $(PKG_BUILD_DIR)/scripts/defaults $(1)/usr/share/icwmp
	$(CP) $(PKG_BUILD_DIR)/scripts/functions $(1)/usr/share/icwmp
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/icwmp.sh $(1)/usr/sbin/icwmp
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/iwepkeygen $(1)/usr/sbin/iwepkeygen
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/watchicwmpd.sh $(1)/usr/sbin/watchicwmpd
endif
endef

define Package/icwmp-$(BUILD_VARIANT)/postinst
	#!/bin/sh
	echo "$(CWMP_BKP_FILE)" >> $${IPKG_INSTROOT}/etc/sysupgrade.conf
	if [ -z "$${IPKG_INSTROOT}" ]; then
		echo "Enabling rc.d symlink for icwmpd"
		/etc/init.d/icwmpd enable
	fi
	exit 0
endef

define Package/icwmp-$(BUILD_VARIANT)/prerm
	#!/bin/sh
	if [ -z "$${IPKG_INSTROOT}" ]; then
		echo "Disabling rc.d symlink for icwmpd"
		/etc/init.d/icwmpd disable
	fi
	exit 0
endef


$(eval $(call BuildPackage,icwmp-curl))
$(eval $(call BuildPackage,icwmp-zstream))
