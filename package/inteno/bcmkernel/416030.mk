#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

# update this based on the Broadcom SDK version, 4.16L.03  -> 416030
BRCM_SDK_VERSION:=416030

PKG_NAME:=bcmkernel-3.4
PKG_VERSION:=4.16
PKG_RELEASE:=$(BRCM_SDK_VERSION)

PKG_SOURCE_URL:=git@iopsys.inteno.se:bcmkernel-4.16L.03
PKG_SOURCE_PROTO:=git

PKG_SOURCE_VERSION:=c5d55a3fd0312cfdfa020755778e061c4a3404d2
PKG_SOURCE:=$(PKG_NAME)-$(BRCM_SDK_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/image.mk
include $(INCLUDE_DIR)/kernel.mk

export CONFIG_BCM_CHIP_ID
export CONFIG_BCM_CFE_PASSWORD
export CONFIG_BCM_KERNEL_PROFILE
export CONFIG_SECURE_BOOT_CFE


BCM_BS_PROFILE = $(shell echo $(CONFIG_BCM_KERNEL_PROFILE) | sed s/\"//g)

BCM_KERNEL_VERSION:=3.4.11-rt19
BCM_SDK_VERSION:=bcm963xx
RSTRIP:=true


define Package/bcmkernel/removevoice
	touch $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
endef

ifeq ($(CONFIG_BCM_ENDPOINT_MODULE),y)
define Package/bcmkernel/removevoice
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/endpointdd.ko
endef
endif

define Package/bcmkernel/removesound
	touch $(1)/lib/modules/$(BCM_KERNEL_VERSION)/snd
	touch $(1)/lib/modules/$(BCM_KERNEL_VERSION)/soundcore.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/snd*
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/soundcore.ko
endef

ifeq ($(BCM_USBSOUND_MODULES),y)
define Package/bcmkernel/removesound
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/snd*
endef
endif


define Package/bcmkernel/removei2c
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/i2c*
endef

ifeq ($(CONFIG_BCM_I2C),y)
define Package/bcmkernel/removei2c
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/i2c*
endef
endif

define Package/bcmkernel/removebluetooth
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bluetooth.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bnep.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/btusb.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/rfcomm.ko
	rm $(1)/lib/modules/$(BCM_KERNEL_VERSION)/hci_uart.ko
endef

ifeq ($(CONFIG_BCM_BLUETOOTH),y)
define Package/bcmkernel/removebluetooth
	echo not removing $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bluetooth.ko etc...
endef
endif

define Package/bcmkernel/install
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
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/bcm963xx/userspace/private/apps/vodsl/voip/inc


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
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/userspace/private/apps/vodsl/voip/inc/tpProfiles.h $(STAGING_DIR)/usr/include/bcm963xx/userspace/private/apps/vodsl/voip/inc
	echo "#define BCM_SDK_VERSION $(BRCM_SDK_VERSION)" > $(STAGING_DIR)/usr/include/bcm_sdk_version.h

	# create symlink to kernel build directory
	rm -f $(BUILD_DIR)/bcmkernel
	ln -sfn $(PKG_SOURCE_SUBDIR) $(BUILD_DIR)/bcmkernel

	# Install binaries
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

	# Install libraries
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/*		$(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/gpl/*	$(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/private/*	$(1)/usr/lib/

	rm -f $(1)/usr/lib/ld-uClibc.so.0
	rm -f $(1)/usr/lib/libc.so.0
	rm -f $(1)/usr/lib/libdl.so.0
	rm -f $(1)/usr/lib/libgcc_s.so.1
	rm -f $(1)/usr/lib/libpthread.so.0
	rm -f $(1)/usr/lib/libm.so.0
	rm -f $(1)/usr/lib/libutil.so.0
	rm -f $(1)/usr/lib/libcms_boardctl.so
	rm -f $(1)/usr/lib/libcms_msg.so
	rm -f $(1)/usr/lib/libcms_util.so
	rm -f $(1)/usr/lib/libnvram.so
	rm -f $(1)/usr/lib/libcrypt.so.0
	rm -f $(1)/usr/lib/libbcm_crc.so
	rm -f $(1)/usr/lib/libbcm_flashutil.so

	rm -f $(1)/usr/lib/libcrypto.so
	ln -s /usr/lib/libcrypto.so.1.0.0 $(1)/usr/lib/libcrypto.so
	rm -f $(1)/usr/lib/libcrypto.so.0.9.7
	ln -s /usr/lib/libcrypto.so.1.0.0 $(1)/usr/lib/libcrypto.so.0.9.7
	rm -f $(1)/usr/lib/libssl.so
	ln -s /usr/lib/libssl.so.1.0.0 $(1)/usr/lib/libssl.so
	rm -f $(1)/usr/lib/libssl.so.0.9.7
	ln -s /usr/lib/libssl.so.1.0.0 $(1)/usr/lib/libssl.so.0.9.7

	$(CP) $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/public/*	$(1)/usr/lib/

	rm -rf $(1)/usr/lib/modules
	rm -rf $(1)/usr/lib/private
	rm -rf $(1)/usr/lib/public
	rm -rf $(1)/usr/lib/gpl

	# Install kernel modules
	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/*
	mkdir -p $(1)/lib/
	mkdir -p $(1)/lib/modules/
	mkdir -p $(1)/lib/modules/$(BCM_KERNEL_VERSION)/
	mkdir -p $(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/extra/*	$(1)/lib/modules/$(BCM_KERNEL_VERSION)/extra/
	find $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/lib/modules/$(BCM_KERNEL_VERSION)/kernel/ -name *.ko -exec cp {} $(1)/lib/modules/$(BCM_KERNEL_VERSION)/ \;

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/wlan/*			$(1)/etc/wlan
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/$(BCM_BS_PROFILE)/fs/etc/telephonyProfiles.d		$(1)/etc/

#	rm -rf $(1)/lib/modules/$(BCM_KERNEL_VERSION)/bcm_usb.ko

	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/kernel/linux-3.4rt/vmlinux $(KDIR)/vmlinux.bcm.elf
	$(KERNEL_CROSS)strip --remove-section=.note --remove-section=.comment $(KDIR)/vmlinux.bcm.elf
	$(KERNEL_CROSS)objcopy $(OBJCOPY_STRIP) -O binary $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/kernel/linux-3.4rt/vmlinux $(KDIR)/vmlinux.bcm

	# bootloader nor
#	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_rom/bcm9$(CONFIG_BCM_CHIP_ID)_cfe.w $(KDIR)/bcm_bootloader_cfe.w

	# ram part of the bootloader for nand boot
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_ram/cfe$(CONFIG_BCM_CHIP_ID).bin $(KDIR)/cferam.001
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/cfe/build/broadcom/bcm63xx_rom/cfe$(CONFIG_BCM_CHIP_ID)_nand.v $(KDIR)/cfe$(CONFIG_BCM_CHIP_ID)_nand.v
	cp -R $(PKG_BUILD_DIR)/$(BCM_SDK_VERSION)/targets/cfe/ $(KDIR)/cfe
#	dd if=$(KDIR)/vmlinux.bcm.elf of=$(KDIR)/vmlinux.bcm bs=4096 count=1
#	$(KERNEL_CROSS)objcopy $(OBJCOPY_STRIP) -S $(LINUX_DIR)/vmlinux $(KERNEL_BUILD_DIR)/vmlinux.elf

	$(call Package/bcmkernel/removevoice,$(1))
	$(call Package/bcmkernel/removesound,$(1))
#	$(call Package/bcmkernel/removei2c,$(1))
endef

