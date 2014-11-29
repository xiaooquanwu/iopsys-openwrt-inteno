#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=statd
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

target=$(firstword $(subst -, ,$(BOARD)))

TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib -luci -lubus -lblobmsg_json

TARGET_CFLAGS += $(FPIC) -Dtarget_$(target)=1 -Wall
MAKE_FLAGS += TARGET="$(target)"

define Package/statd
  CATEGORY:=Utilities
  TITLE:=Statistics manager
  DEPENDS:=+libuci +ubus +libblobmsg-json
endef

define Package/statd/description
	Application that listen on ubus events to be sent on syslog or snmp
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include"
endef

define Package/statd/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) ./files/* $(1)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/statd $(1)/sbin/
endef

$(eval $(call BuildPackage,statd))
