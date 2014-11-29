#
# Copyright (C) 2013 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libucix
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

target=$(firstword $(subst -, ,$(BOARD)))

        -Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
		-Wl,-rpath-link=$(STAGING_DIR)/lib -luci

TARGET_LDFLAGS += -shared -luci -Wl,-soname
TARGET_CFLAGS += $(FPIC) -Dtarget_$(target)=1 -Wall
MAKE_FLAGS += TARGET="$(target)"

define Package/libucix
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=UCI library
  DEPENDS:=+libuci 
endef

define Package/libucix/description
	UCI helper library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/libucix/install
	$(INSTALL_DIR) $(1)/usr/lib/
	( cd $(PKG_BUILD_DIR) ; rm -f libucix.so && ln -s libucix.so.$(PKG_VERSION) libucix.so )
	$(CP) $(PKG_BUILD_DIR)/libucix.so* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libucix.so* $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/*.h $(STAGING_DIR)/usr/include/
endef

$(eval $(call BuildPackage,libucix))
