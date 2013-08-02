#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=questd
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib -luci -lubus -lubox -lpthread

TARGET_CFLAGS += $(FPIC)

define Package/questd
  CATEGORY:=Utilities
  DEPENDS:=+libuci +libubox +ubus +netifd
  TITLE:=router info daemon
endef

define Package/questd/description
	questd keeps the router's system and network information and presents
	this information via ubus
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include"
endef

define Package/questd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/questd $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,questd))
