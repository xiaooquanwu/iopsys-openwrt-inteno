#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk


PKG_NAME:=speedtest-cli
PKG_RELEASE:=1
PKG_SOURCE:=speedtest_cli.py
PKG_SOURCE_URL:=https://raw.githubusercontent.com/sivel/speedtest-cli/master

include $(INCLUDE_DIR)/package.mk

define Package/speedtest-cli
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=speedtest-cli utility
  DEPENDS:=+python +python-mini +python-openssl +python-expat
  URL:=https://github.com/sivel/speedtest-cli
endef

define Package/speedtest-cli/description
  Command line interface for testing internet bandwidth using speedtest.net.
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/speedtest-cli/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(DL_DIR)/speedtest_cli.py $(1)/sbin/
	chmod 744 $(1)/sbin/speedtest_cli.py
endef

$(eval $(call BuildPackage,speedtest-cli))
