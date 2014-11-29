include $(TOPDIR)/rules.mk

PKG_NAME:=tptest
PKG_RELEASE:=0
PKG_VERSION:=1.3

include $(INCLUDE_DIR)/package.mk

TARGET_LDFLAGS+= \
        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib

TARGET_CFLAGS+= \
		-DUNIX -DLINUX

MAKE_OPTS:= \
    ARCH="$(LINUX_KARCH)" \
    CROSS_COMPILE="$(TARGET_CROSS)" \
    SUBDIRS="$(PKG_BUILD_DIR)" \

define Package/tptest
  CATEGORY:=Utilities
  TITLE:=TPTEST speed test utility
endef

define Package/tptest/description
	TPTEST speed test utility
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) MAKE_OPTS=$(MAKE_OPTS) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include -I$(STAGING_DIR)/usr/include"
endef

define Package/tptest/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tptest $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,tptest))
