# 
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
IHGSP_VERSION:=$(CONFIG_TARGET_MODEL_NAME)_$(CONFIG_TARGET_CUSTOMER)$(CONFIG_TARGET_VERSION)-$(shell date '+%y%m%d_%H%M')
IHGSP_BUILD_VERSION:=$(shell echo $(IHGSP_VERSION) | sed s/\"//g)
#BUILD_DIR_ROOT=$(BUILD_DIR)/root-brcm63xx_ref_nand
KERNEL_ENTRY:= $(shell readelf -h $(KDIR)/vmlinux.bcm.elf | grep Entry | awk '{print $$4}')

define Image/Build/REF
	
	#add jffs2 eof marker
	cat $(KDIR)/jffs2_eof_marker_128k >> $(KDIR)/root.$(1)

	# Tag the rootfs image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_fs_image \
                -c $(2) \
		-q NAND128 \
		-L

	# Concat the cfe and rootfs together
	cat $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_cfe $(KDIR)/root.$(1) > $(KDIR)/cfe_fs_image_128

	# Tag the combined image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe_fs_image_128 \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION).w \
                -c $(2) \
		-q NAND128 \
		-L

endef

define Image/Build/CFE
	# Tag the cfe image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe$(2)_nand.v \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_cfe \
                -c $(2) \
		-q NAND128 \
		-L
endef


define Image/Prepare
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -lp2 -lc1 $(KDIR)/vmlinux.bcm $(KDIR)/vmlinux.lzma
	$(STAGING_DIR_HOST)/bin/imagetag_nand -L -K -i $(KDIR)/vmlinux.lzma -o $(KDIR)/vmlinux.lz -e $(KERNEL_ENTRY) -l $(KERNEL_ENTRY)

	dd if=/dev/zero of=$(KDIR)/file128k.tmp bs=1k count=128

	#generate jffs2 eof file marker of 128k size
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/jffs2_eof_marker
	#generate board tag
	cat $(KDIR)/jffs2_eof_marker $(KDIR)/file128k.tmp | head --bytes=1024 > $(KDIR)/jffs2_eof_marker_1024
	echo -ne $(CONFIG_TARGET_MODEL_NAME) >>$(KDIR)/jffs2_eof_marker_1024
	cat $(KDIR)/jffs2_eof_marker_1024 $(KDIR)/file128k.tmp | head --bytes=2048 > $(KDIR)/jffs2_eof_marker_2048
	#generate customer tag
	echo -ne $(CONFIG_TARGET_CUSTOMER) >>$(KDIR)/jffs2_eof_marker_2048
	cat $(KDIR)/jffs2_eof_marker_2048 $(KDIR)/file128k.tmp | head --bytes=131072 > $(KDIR)/jffs2_eof_marker_128k

	#these files need to go in a set of erase blocks of their own to minimize the chance of relocation
	echo "/cferam.001"    >  $(KDIR)/sortlist
	echo "/vmlinux.lz"    >> $(KDIR)/sortlist

	#add the files that should not be compressed
	echo -e "/IOP3"       >  $(KDIR)/nocomprlist
	echo -e "/cferam.001" >> $(KDIR)/nocomprlist
	echo -e "/vmlinux.lz" >> $(KDIR)/nocomprlist
	touch $(TARGET_DIR)/IOP3
	cp $(KDIR)/vmlinux.lz $(TARGET_DIR)/
	cp $(KDIR)/cferam.001 $(TARGET_DIR)/

	touch -c $(TOPDIR)/target/linux/$(BOARD)/base-files/etc/banner
	cp $(TOPDIR)/target/linux/$(BOARD)/base-files/etc/banner $(TARGET_DIR)/etc/banner
	sed -i '/^IOP Version:/d' $(TARGET_DIR)/etc/banner
	sed -i '7iIOP Version: $(IHGSP_BUILD_VERSION)' $(TARGET_DIR)/etc/banner
	mkdir $(TARGET_DIR)/lib/db/version
	echo $(IHGSP_BUILD_VERSION)  > $(TARGET_DIR)/lib/db/version/iop_version
#	echo $(CONFIG_TARGET_IBOARDID)> $(TARGET_DIR)/lib/db/version/iop_iboardid
	echo $(CONFIG_TARGET_CUSTOMER)> $(TARGET_DIR)/lib/db/version/iop_customer

	#strip out unneeded files
	rm -rf $(TARGET_DIR)/lib/modules/3.3.8
	rm -rf $(TARGET_DIR)/usr/lib/libvorbisenc*
endef

define Image/Build
	$(call Image/Build/CFE,,$(CONFIG_BCM_CHIP_ID))
	$(call Image/Build/REF,$(1),$(CONFIG_BCM_CHIP_ID))
endef

$(eval $(call BuildImage))
