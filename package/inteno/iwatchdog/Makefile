#
# Copyright (C) 2013 Inteno
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=iwatchdog
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

# support parallel build
PKG_BUILD_PARALLEL:=1

#re create configure scripts if not present.
PKG_FIXUP:=autoreconf

# run install target when cross compiling. basically, make install DESTDIR=$(PKG_INSTALL_DIR)
# this way we don't need to pick out the resulting files from the build dir.
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/iwatchdog
  CATEGORY:=Utilities
  TITLE:=Inteno watchdog
endef

define Package/iwatchdog/description
	Watchdog application for bradcom /proc/watchdog API.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Package/iwatchdog/install

	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/iwatchdog $(1)/sbin/

	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/etc/init.d/* $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,iwatchdog))
