#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ami
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
#STAMP_PREPARED := $(STAMP_PREPARED)_$(call confvar,CONFIG_MTD_REDBOOT_PARTS)

include $(INCLUDE_DIR)/package.mk

target=$(firstword $(subst -, ,$(BOARD)))

TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib -luci -lubus -lubox -lpthread

TARGET_CFLAGS += $(FPIC) -Dtarget_$(target)=1 -Wall
MAKE_FLAGS += TARGET="$(target)"

define Package/ami
  CATEGORY:=Utilities
  DEPENDS:= +libuci +libubox +ubus
  TITLE:=Asterisk management interface listener
endef

define Package/ami/description
	Application that listen on the asterisk management interface to handle the leds
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include"
endef

define Package/ami/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) ./files/* $(1)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ami_tool $(1)/sbin/
endef

$(eval $(call BuildPackage,ami))
