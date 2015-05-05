#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=questd
PKG_VERSION:=2.0.3
PKG_RELEASE:=3
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

ifeq ($(CONFIG_PACKAGE_bcmkernel),y)
BCMKERNEL_DIR:=$(BUILD_DIR)/bcmkernel/bcm963xx
else
BCMKERNEL_DIR:=$(BUILD_DIR)/bcmopen-consumer/bcm963xx
endif
export BCMKERNEL_DIR

define Package/questd
  CATEGORY:=Utilities
  DEPENDS:=+libuci +libubox +ubus +libpthread
  TITLE:=router info daemon
endef

define Package/questd/description
	questd collects system and network information and presents
	this information via ubus
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/questd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/tmp
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/questd $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,questd))
