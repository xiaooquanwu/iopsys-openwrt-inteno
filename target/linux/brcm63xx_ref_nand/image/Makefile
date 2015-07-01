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
BUILD_DIR_ROOT=$(BUILD_DIR)/root-brcm63xx_ref_nand
KERNEL_ENTRY:= $(shell readelf -h $(KDIR)/vmlinux.bcm.elf | grep Entry | awk '{print $$4}')

CFE_BUILT:=
BOARD_ID:=D301

UBIFS_OPTS = -v -c 2048 -m 2048 -e 126976 -x lzma --squash-uids
UBINIZE_OPTS = -v -m 2048 -p 128KiB

ifeq ($(CONFIG_BCM_CHIP_ID),"63138")
MKFS_JFFS2_ARGS      := --little-endian
IMAGETAG_KERNEL_ARGS := -L -K -i $(KDIR)/vmlinux.lzma -o $(KDIR)/vmlinux.lz -e $(KERNEL_ENTRY) -l $(KERNEL_ENTRY)
IMAGETAG_IMAGE_ARGS  := -c $(CONFIG_BCM_CHIP_ID) -q NAND128 -L
else
MKFS_JFFS2_ARGS      := --big-endian
IMAGETAG_KERNEL_ARGS := -K -i $(KDIR)/vmlinux.lzma -o $(KDIR)/vmlinux.lz -e $(KERNEL_ENTRY)
IMAGETAG_IMAGE_ARGS  := -c $(CONFIG_BCM_CHIP_ID) -q NAND128 -b $(BOARD_ID)
endif


# NOTE:
#
# The following targets generate a number of different images:
# .w = Broadcom style JFFS2 image for old banked lay-out
# .x = Broadcom style UBIFS image for use with CFE, containing CFE, nvram,
#      kernel and initial UBI image
# .y = UBI system upgrade image, containing cfe, kernel & UBIFS data
# .z = System upgrade image containing CFE only
#
# All image types have an image tag attached to the end. The tag includes
# compatibility information and a CRC of the complete image.
#
# The .w and .x images contain a complete boot block (eraseblock 0) image
# followed by N complete eraseblocks of JFFS2 or nvram+JFFS2+UBI data and
# finally board and cusomer tags.
#
# The .y and .z images use a header containing the size of each component that
# is included (or zero for components not included). The header is followed
# by the included components in a fixed predefined order.
#

define Image/Prepare

	# Called before any rootfs images are built
	echo "In Image/prepare $(1) $(2)"

	# compressed kernel (.w.x.y)
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -lp2 -lc1 $(KDIR)/vmlinux.bcm $(KDIR)/vmlinux.lzma
	$(STAGING_DIR_HOST)/bin/imagetag_nand $(IMAGETAG_KERNEL_ARGS)

    ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
	# make kernel jffs2 image (.x.y)
	mkdir -p $(KDIR)/kernel_fs
	cp $(KDIR)/vmlinux.lz $(KDIR)/kernel_fs/
	touch $(KDIR)/kernel_fs/cferam.001
	echo "/cferam.001" > $(KDIR)/nocomprlist
	echo "/vmlinux.lz" >> $(KDIR)/nocomprlist
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 \
		$(MKFS_JFFS2_ARGS) \
		-v -e 128KiB --squash --no-cleanmarkers \
		--pad -N $(KDIR)/nocomprlist \
		-d $(KDIR)/kernel_fs -o $(KDIR)/vmlinux.jffs2
    endif

	# generate jffs2 eof file marker of 128k size (.w)
	dd if=/dev/zero of=$(KDIR)/file128k.tmp bs=1k count=128
	echo -ne '\xde\xad\xc0\xde' > $(KDIR)/jffs2_eof_marker
	#generate board tag
	cat $(KDIR)/jffs2_eof_marker $(KDIR)/file128k.tmp | head --bytes=1024 > $(KDIR)/jffs2_eof_marker_1024
	echo -ne $(CONFIG_TARGET_MODEL_NAME) >>$(KDIR)/jffs2_eof_marker_1024
	cat $(KDIR)/jffs2_eof_marker_1024 $(KDIR)/file128k.tmp | head --bytes=2048 > $(KDIR)/jffs2_eof_marker_2048
	#generate customer tag
	echo -ne $(CONFIG_TARGET_CUSTOMER) >>$(KDIR)/jffs2_eof_marker_2048
	cat $(KDIR)/jffs2_eof_marker_2048 $(KDIR)/file128k.tmp | head --bytes=131072 > $(KDIR)/jffs2_eof_marker_128k

	# these files need to go in a set of erase blocks of their own to minimize the chance of relocation (.w)
	echo "/cferam.001"    >  $(KDIR)/sortlist
	echo "/vmlinux.lz"    >> $(KDIR)/sortlist

	# add the files that should not be compressed (.w)
	echo -e "/IOP3"       > $(KDIR)/nocomprlist
	echo -e "/cferam.001" >> $(KDIR)/nocomprlist
	echo -e "/vmlinux.lz" >> $(KDIR)/nocomprlist

	# add "first boot" marker (.w.x.y)
	touch $(TARGET_DIR)/IOP3

	# add kernel and cferam flag (.w)
	touch $(TARGET_DIR)/cferam.001
	cp $(KDIR)/vmlinux.lz $(TARGET_DIR)/

	# add/set version info (.w.x.y)
	#touch -c $(TOPDIR)/target/linux/brcm63xx_ref_nand/base-files/etc/banner
	#cp $(TOPDIR)/target/linux/brcm63xx_ref_nand/base-files/etc/banner $(BUILD_DIR_ROOT)/etc/banner
	sed -i 's/^IOP Version.*/IOP Version: $(IHGSP_BUILD_VERSION)/gi' $(BUILD_DIR_ROOT)/etc/banner
	sed -i 's/^BrcmRef Base.*/BrcmRef Base: bcm963xx_4.16L.03/gi' $(BUILD_DIR_ROOT)/etc/banner
	#sed -i '7iIOP Version: $(IHGSP_BUILD_VERSION)' $(BUILD_DIR_ROOT)/etc/banner
	mkdir -p $(BUILD_DIR_ROOT)/lib/db/version
	echo $(IHGSP_BUILD_VERSION)  > $(BUILD_DIR_ROOT)/lib/db/version/iop_version
#	echo $(CONFIG_TARGET_IBOARDID)> $(BUILD_DIR_ROOT)/lib/db/version/iop_iboardid
	echo $(CONFIG_TARGET_CUSTOMER)> $(BUILD_DIR_ROOT)/lib/db/version/iop_customer

	# strip out unneeded files (.w.x.y)
	rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.3.8
	rm -rf $(BUILD_DIR_ROOT)/usr/lib/libvorbisenc*

    ifeq ($(CONFIG_TARGET_IBOARDID),"VOX25")
		rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.4.11-rt19/bluetooth.ko
		rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.4.11-rt19/btusb.ko
		rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.4.11-rt19/extra/dect.ko
		rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.4.11-rt19/extra/dectshim.ko
		rm -rf $(BUILD_DIR_ROOT)/lib/modules/3.4.11-rt19/extra/dhd.ko
		rm -rf $(BUILD_DIR_ROOT)/etc/wlan/dhd
    endif
endef

define Image/Build/Header
	# Args: $1=header $2=cfe $3=kernel $4=ubifs
	# Any unused arguments should be passed as 'void'

	echo "In Image/Build/Header $(1) $(2) $(3) $(4)"

	truncate --size=0 $(KDIR)/void
	echo "IntenoBlob" > $(KDIR)/hdr
	echo "integrity MD5SUM" >> $(KDIR)/hdr
	echo "board $(BOARD_ID)" >> $(KDIR)/hdr
	echo "chip $(CONFIG_BCM_CHIP_ID)" >> $(KDIR)/hdr
	echo "model $(CONFIG_TARGET_MODEL_NAME)" >> $(KDIR)/hdr
	echo "customer $(CONFIG_TARGET_CUSTOMER)" >> $(KDIR)/hdr
	echo "cfe $$$$(ls -l $(KDIR)/$(2) | awk '{print $$$$5}')" >> $(KDIR)/hdr
	echo "vmlinux $$$$(ls -l $(KDIR)/$(3) | awk '{print $$$$5}')" >> $(KDIR)/hdr
	echo "ubifs $$$$(ls -l $(KDIR)/$(4) | awk '{print $$$$5}')" >> $(KDIR)/hdr
	cat $(KDIR)/hdr /dev/zero | head --bytes=1024 > $(KDIR)/$(1)
endef

define Image/Build/CFE

	echo "In Image/Build/CFE $(1) $(2)"

	# Tag the (old style) cfe image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe$(CONFIG_BCM_CHIP_ID)_nand.v \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_cfe \
		$(IMAGETAG_IMAGE_ARGS)

    ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
	# Make header (.z)
	$(call Image/Build/Header,header.z,cfe$(CONFIG_BCM_CHIP_ID)_nand.v,void,void)

	# Concat header and cfe together (.z)
	cat $(KDIR)/header.z \
	    $(KDIR)/cfe$(CONFIG_BCM_CHIP_ID)_nand.v \
	    > $(BIN_DIR)/$(IHGSP_BUILD_VERSION).z

	# Attach checksum to combined image (.z)
	md5sum $(BIN_DIR)/$(IHGSP_BUILD_VERSION).z |awk '{printf "%s",$$$$1}' \
		>> $(BIN_DIR)/$(IHGSP_BUILD_VERSION).z
    endif

	$(eval CFE_BUILT:=1)
endef

define Image/Build/jffs2-128k

	echo "In Image/Build/jffs2-128k $(1) $(2)"

	#add jffs2 eof marker
	cat $(KDIR)/jffs2_eof_marker_128k >> $(KDIR)/root.$(1)

	# Tag the rootfs image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_fs_image \
		$(IMAGETAG_IMAGE_ARGS)

	# Concat the cfe and rootfs together (.w)
	cat $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_cfe \
	    $(KDIR)/root.$(1) > $(KDIR)/cfe_fs_image_128

	# Tag the combined image (.w)
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe_fs_image_128 \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION).w \
		$(IMAGETAG_IMAGE_ARGS)

	# DIRTY! DIRTY! DIRTY! DIRTY! DIRTY! DIRTY! DIRTY!
	# kernel and cferam flag not needed in UBI file systems!
	# Take these away in "prepare" when we no longer build JFFS2 rootfs.
	rm $(TARGET_DIR)/cferam.001
	rm $(TARGET_DIR)/vmlinux.lz
endef

define Image/Build/ubi
	# This function will only be called if CONFIG_TARGET_ROOTFS_UBIFS = y

	echo "In Image/Build/ubi $(1) $(2)"

	cat /dev/zero | tr "\000" "\377" \
		| head --bytes=$$$$((84*131072)) > $(KDIR)/lotsof.ff

	# make nvram2 image (.x)
	echo 'NVRAM' > $(KDIR)/nvram_marker
	cat $(KDIR)/nvram_marker $(KDIR)/lotsof.ff \
		| head --bytes=131072 > $(KDIR)/nvram.bin

	# Concat cfe, nvram, vmlinux and ubi images together (.x)
	cat $(BIN_DIR)/$(IHGSP_BUILD_VERSION)_cfe \
	    $(KDIR)/nvram.bin \
	    $(KDIR)/nvram.bin \
	    $(KDIR)/nvram.bin \
	    $(KDIR)/vmlinux.jffs2 \
	    $(KDIR)/lotsof.ff \
	    | head --bytes=$$$$((84*131072)) > $(KDIR)/temp_x.img
	cat $(KDIR)/temp_x.img \
	    $(KDIR)/root.ubi > $(KDIR)/cfe_nvram_vmlinux_ubi_image_128

	rm $(KDIR)/temp_x.img $(KDIR)/lotsof.ff

	# Tag the combined image (.x)
	$(STAGING_DIR_HOST)/bin/imagetag_nand \
		-f $(KDIR)/cfe_nvram_vmlinux_ubi_image_128 \
		-o $(BIN_DIR)/$(IHGSP_BUILD_VERSION).x \
		$(IMAGETAG_IMAGE_ARGS)

	# Make header (.y)
	$(call Image/Build/Header,header.y,cfe$(CONFIG_BCM_CHIP_ID)_nand.v,vmlinux.jffs2,root.ubifs)

	# Concat header, cfe, vmlinux and ubifs together (.y)
	cat $(KDIR)/header.y \
	    $(KDIR)/cfe$(CONFIG_BCM_CHIP_ID)_nand.v \
	    $(KDIR)/vmlinux.jffs2 \
	    $(KDIR)/root.ubifs > $(BIN_DIR)/$(IHGSP_BUILD_VERSION).y

	# Attach checksum to combined image (.y)
	md5sum $(BIN_DIR)/$(IHGSP_BUILD_VERSION).y |awk '{printf "%s",$$$$1}' \
		>> $(BIN_DIR)/$(IHGSP_BUILD_VERSION).y
endef

define Image/Build

	# Called at the end of Image/mkfs/{jffs2,squashfs,ubifs} depending on
	# which CONFIG_TARGET_ROOTFS_* are enabled in the configuration.
	echo "In Image/Build $(1) $(2)"

	$(if $(CFE_BUILT),,$(call Image/Build/CFE))
	$(call Image/Build/$(1),$(1))
endef


$(eval $(call BuildImage))
