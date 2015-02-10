#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bcmopen
PKG_VERSION:=consumer
PKG_RELEASE:=1

TARGET_VERSION=$(shell echo $(CONFIG_TARGET_VERSION) | sed s/\"//g)
TARGET_IBOARDID=$(shell echo $(CONFIG_TARGET_IBOARDID) | sed s/\"//g)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(TARGET_VERSION).$(TARGET_IBOARDID).tar.gz
PKG_SOURCE_URL:=http://ihgsp.inteno.se/ihgsp/consumer/
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

define Package/bcmopen
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Broadcom sdk 3.4 kernel
  URL:=
  MENU:=1
endef

define Package/bcmopen/description
	Broadcom kernel and userspace, compiled with the broadcom supplied tool-chain.
endef

define Package/bcmopen/config
	source "$(SOURCE)/Config.in"
endef


define Package/bcmopen/removevoice
	touch $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
endef

ifeq ($(CONFIG_BCM_ENDPOINT_MODULE),y)
define Package/bcmopen/removevoice
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/endpointdd.ko
endef
endif


define Package/bcmopen/removei2c
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/i2c*
endef

ifeq ($(CONFIG_BCM_I2C),y)
define Package/bcmopen/removei2c
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/i2c*
endef
endif

#define Package/bcmkernel/prepare
#endif

define Package/bcmopen/install
	$(INSTALL_DIR) $(1)/lib/bcm
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/adsl
	$(INSTALL_DIR) $(1)/etc/wlan
	$(INSTALL_DIR) $(1)/etc/cms_entity_info.d



# Install header files
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/LinuxUser
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx


	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/shared/opensource/include/bcm963xx/* $(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx

	$(CP) -r $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/opensource/include/bcm963xx/* $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgTypes.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgTypes.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgCountryCfg.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgCountryCfg.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgCountryCfgCustom.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgCountryCfgCustom.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgLogCfgCustom.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgLogCfgCustom.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgLog.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgLog.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/countryArchive.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/countryArchive.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/inc/vrgCountry.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/inc/vrgCountry.h

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/casCtl/inc/casCtl.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/casCtl/inc/casCtl.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/codec/codec.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/codec/codec.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/endpt/inc/endpt.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc/endpt.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/voice_res_gw/endpt/inc/vrgEndpt.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/voice_res_gw/endpt/inc/vrgEndpt.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/LinuxUser/bosTypesLinuxUser.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/LinuxUser/bosTypesLinuxUser.h

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosMutex.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosMutex.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosSpinlock.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosSpinlock.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosMsgQ.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosMsgQ.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosCritSect.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosCritSect.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosTypes.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosTypes.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosTime.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosTime.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosSem.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosSem.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosCfgCustom.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosCfgCustom.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosIpAddr.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosIpAddr.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosTimer.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosTimer.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosError.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosError.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosLog.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosLog.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosSleep.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosSleep.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosMisc.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosMisc.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosCfg.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosCfg.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosEvent.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosEvent.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosTask.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosTask.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosUtil.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosUtil.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosInit.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosInit.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosSocket.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosSocket.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_common/bos/publicInc/bosFile.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_common/bos/publicInc/bosFile.h

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_drivers/inc/xdrvSlic.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc/xdrvSlic.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_drivers/inc/xdrvApm.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc/xdrvApm.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_drivers/inc/xdrvCas.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc/xdrvCas.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/xChange/dslx_common/xchg_drivers/inc/xdrvTypes.h $(STAGING_DIR)/usr/include/bcm963xx/xChange/dslx_common/xchg_drivers/inc/xdrvTypes.h

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/include/bcm963xx/endptvoicestats.h $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx/endptvoicestats.h
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/include/bcm963xx/endpointdrv.h $(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/broadcom/include/bcm963xx/endpointdrv.h

	# create symlink to kernel build directory
	rm -f $(BUILD_DIR)/bcmkernel
	ln -sfn $(PKG_SOURCE_SUBDIR) $(BUILD_DIR)/bcmkernel

###
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/bin/*		$(1)/usr/sbin/

	rm -f $(1)/usr/sbin/dhcp6c
	rm -f $(1)/usr/sbin/dhcp6s
	rm -f $(1)/usr/sbin/dhcpc
	rm -f $(1)/usr/sbin/dhcpd
	rm -f $(1)/usr/sbin/dnsproxy
	rm -f $(1)/usr/sbin/httpd
	rm -f $(1)/usr/sbin/openssl
	rm -f $(1)/usr/sbin/racoon
	rm -f $(1)/usr/sbin/ripd
	rm -f $(1)/usr/sbin/send_cms_msg
	rm -f $(1)/usr/sbin/sshd
	rm -f $(1)/usr/sbin/ssk
	rm -f $(1)/usr/sbin/telnetd
	rm -f $(1)/usr/sbin/tr64c
	rm -f $(1)/usr/sbin/tr69c
	rm -f $(1)/usr/sbin/ubi*
	rm -f $(1)/usr/sbin/udhcpd
	rm -f $(1)/usr/sbin/upnp
	rm -f $(1)/usr/sbin/upnpd
	rm -f $(1)/usr/sbin/vodsl
	rm -f $(1)/usr/sbin/wlmngr
	rm -f $(1)/usr/sbin/zebra


	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/cms_entity_info.d/eid_bcm_kthreads.txt	$(1)/etc/cms_entity_info.d/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/cms_entity_info.d/symbol_table.txt		$(1)/etc/cms_entity_info.d/


	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/*		$(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/gpl/*	$(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/private/*	$(1)/usr/lib/

	mv $(1)/usr/lib/ld-uClibc.so.0		$(1)/lib/bcm/
	mv $(1)/usr/lib/libc.so.0		$(1)/lib/bcm/
	mv $(1)/usr/lib/libdl.so.0		$(1)/lib/bcm/
	mv $(1)/usr/lib/libgcc_s.so.1		$(1)/lib/bcm/
	mv $(1)/usr/lib/libpthread.so.0		$(1)/lib/bcm/

	rm -f $(1)/usr/lib/libcrypt.so.0
	rm -f $(1)/usr/lib/libm.so.0
	rm -f $(1)/usr/lib/libutil.so.0
	rm -f $(1)/usr/lib/libcms_boardctl.so
	rm -f $(1)/usr/lib/libcms_msg.so
	rm -f $(1)/usr/lib/libcms_util.so
	rm -f $(1)/usr/lib/libcrypto.so.0.9.7
	rm -f $(1)/usr/lib/libssl.so.0.9.7
	rm -f $(1)/usr/lib/libnvram.so


	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/public/*	$(1)/usr/lib/

	rm -rf $(1)/usr/lib/modules
	rm -rf $(1)/usr/lib/private
	rm -rf $(1)/usr/lib/public
	rm -rf $(1)/usr/lib/gpl


	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/*
	mkdir -p $(1)/lib/
	mkdir -p $(1)/lib/modules/
	mkdir -p $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	mkdir -p $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra


	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/extra/*			$(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/crypto/*		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/scsi/*		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/usb/serial/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/usb/storage/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/fs/cifs/*		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv4/*.ko		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/dns_resolver/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv4/netfilter/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/ipv6/netfilter/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/net/netfilter/*		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/char/hw_random/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp    $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/i2c/i2c-core.ko		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp    $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/i2c/i2c-dev.ko		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp    $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/i2c/algos/i2c-algo-bit.ko	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp    $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/i2c/busses/i2c-gpio.ko		$(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	cp    $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/drivers/i2c/busses/i2c-gpio-custom.ko	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/

	touch $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/char/adsl/impl1/adsl_phy.bin
#	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/bcmdrivers/broadcom/char/adsl/impl1/adsl_phy.bin		$(1)/etc/adsl/adsl_phy.bin

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/wlan/*_map.bin		$(1)/etc/wlan
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/telephonyProfiles.d		$(1)/etc/

#	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bcm_usb.ko

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

	$(call Package/bcmopen/removevoice,$(1))
	$(call Package/bcmopen/removei2c,$(1))
endef

$(eval $(call BuildPackage,bcmopen))
