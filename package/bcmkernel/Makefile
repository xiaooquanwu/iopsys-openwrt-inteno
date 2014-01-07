#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bcmkernel-3.4
PKG_VERSION:=4.14
PKG_RELEASE:=4

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git@iopsys.inteno.se:bcmkernel-4.14L.04
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=29bee8fda5de318c75f9be8712de1793754502e6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk

export CONFIG_BCM_CHIP_ID
export CONFIG_BCM_KERNEL_PROFILE
export CONFIG_SECURE_BOOT_CFE


BCM_BS_PROFILE = $(shell echo $(CONFIG_BCM_KERNEL_PROFILE) | sed s/\"//g)

BCM_KERNEL_VERSION:=3.4.11-rt19
BCM_SDK_VERSION:=bcm963xx
RSTRIP:=true

define Package/bcmkernel
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Broadcom sdk 3.4 kernel
  URL:=
  MENU:=1
endef

define Package/bcmkernel/description
	Broadcom kernel and userspace, compiled with the broadcom supplied tool-chain.
endef

define Package/bcmkernel/config
	source "$(SOURCE)/Config.in"
endef


define Package/bcmkernel/removevoice
	touch $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
endef

ifeq ($(CONFIG_BCM_ENDPOINT_MODULE),y)
define Package/bcmkernel/removevoice
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
endef
endif

#define Package/bcmkernel/prepare
#endif

define Package/bcmkernel/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/adsl
	$(INSTALL_DIR) $(1)/etc/wlan



# Install header files
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc

	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/include/bcm963xx/* $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/opensource/include/bcm963xx/* $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/endpt/inc/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/codec/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/casCtl/inc/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc
	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_drivers/inc/* $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc


	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/bin/* $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/gpl/* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/private/* $(1)/usr/lib/

	rm -f $(1)/usr/lib/libcms_boardctl.so
	rm -f $(1)/usr/lib/libcms_msg.so
	rm -f $(1)/usr/lib/libcms_util.so
	rm -f $(1)/usr/lib/libcrypto.so.0.9.7
	rm -f $(1)/usr/lib/libssl.so.0.9.7
	rm -f $(1)/usr/sbin/vodsl

	rm -f $(1)/usr/sbin/httpd
	rm -f $(1)/usr/sbin/openssl
	rm -f $(1)/usr/sbin/telnetd

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/public/* $(1)/usr/lib/

	rm -rf $(1)/usr/lib/modules
	rm -rf $(1)/usr/lib/private
	rm -rf $(1)/usr/lib/public
	rm -rf $(1)/usr/lib/gpl


	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/*
	mkdir $(1)/lib/
	mkdir $(1)/lib/modules/
	mkdir $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	mkdir $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra


	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/extra/*                          $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/crypto/*                  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/scsi/*            $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/usb/serial/*	  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/usb/storage/*	  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv4/*.ko		  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/	
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv4/netfilter/*      $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv6/netfilter/*      $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/netfilter/*           $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/fs/cifs/*		  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
#	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/char/hw_random/*  $(1)/lib/modules/$(BCM_KERNEL_VERSION)/


	touch $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/char/adsl/impl1/adsl_phy.bin
#	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/char/adsl/impl1/adsl_phy.bin                                      $(1)/etc/adsl/adsl_phy.bin

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/wlan/*_map.bin	$(1)/etc/wlan

#	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bcm_usb.ko
	rm -rf $(1)/usr/lib/libnvram.so

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/kernel/linux-3.4rt/vmlinux $(KDIR)/vmlinux.bcm.elf
	$(KERNEL_CROSS)strip --remove-section=.note --remove-section=.comment $(KDIR)/vmlinux.bcm.elf
	$(KERNEL_CROSS)objcopy $(OBJCOPY_STRIP) -O binary $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/kernel/linux-3.4rt/vmlinux $(KDIR)/vmlinux.bcm

# bootloader nor
#	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_rom/bcm9$(CONFIG_BCM_CHIP_ID)_cfe.w $(KDIR)/bcm_bootloader_cfe.w
# ram part of the bootloader for nand boot
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_ram/cfe$(CONFIG_BCM_CHIP_ID).bin $(KDIR)/cferam.000
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_rom/cfe$(CONFIG_BCM_CHIP_ID)_nand.v $(KDIR)/cfe$(CONFIG_BCM_CHIP_ID)_nand.v
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/cfe/ $(KDIR)/cfe
#	dd if=$(KDIR)/vmlinux.bcm.elf of=$(KDIR)/vmlinux.bcm bs=4096 count=1
#	$(KERNEL_CROSS)objcopy $(OBJCOPY_STRIP) -S $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/vmlinux.elf

	$(call Package/bcmkernel/removevoice,$(1))
endef

$(eval $(call BuildPackage,bcmkernel))
