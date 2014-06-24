#
# Copyright (C) 2014 Inteno
#

include $(TOPDIR)/rules.mk

PKG_NAME:=endptcfg
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

#  DEPENDS:= +bcmkernel
define Package/endptcfg
  CATEGORY:=Utilities
  DEPENDS:= +libpthread
  TITLE:=Broadcom endpoint initializer
endef

ifeq ($(CONFIG_TARGET_IBOARDID),"D301")
  TARGET_CFLAGS += -DNTR_SUPPORT
endif

define Package/endptcfg/description
	Broadcom endpoint initializer
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) LINUX_DIR=$(LINUX_DIR) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS) \
			-I$(LINUX_DIR)/include \
			-I$(STAGING_DIR)/usr/include \
			-DRS_ENDIAN_TYPE=RS_BIG_ENDIAN \
			-DBOS_OS_LINUXUSER -DBOS_CFG_TIME \
			-I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/userspace/private/apps/vodsl/voip/inc/ \
			-I$(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/LinuxUser/ \
			$(BUILD_DIR)/bcmkernel-3.4-4.14/bcm963xx/userspace/private/apps/vodsl/telephonyProfiles/telephonyProfiles.o \
			$(BUILD_DIR)/bcmkernel-3.4-4.14/bcm963xx/userspace/private/apps/vodsl/voip/util/log/vodslLog.o \
			$(BUILD_DIR)/bcmkernel-3.4-4.14/bcm963xx/userspace/private/apps/vodsl/endpoint/endpoint_user.o \
			$(BUILD_DIR)/bcmkernel-3.4-4.14/bcm963xx/userspace/private/apps/vodsl/bos/bos.o \
			-lpthread"
endef

define Package/endptcfg/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/endptcfg $(1)/sbin/
endef

$(eval $(call BuildPackage,endptcfg))
