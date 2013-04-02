# 
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_ENTRY:= $(shell readelf -h $(KDIR)/vmlinux.bcm.elf | grep Entry | awk '{print $$4}')

define Image/Build/REF
	# Generate the tagged image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -i $(KDIR)/root.$(1) -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/IOPSYS-$(4).w \
		-b $(2) \
                -c $(3) \
		-q NANDFS128
endef


define Image/Prepare
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -lp2 -lc1 $(KDIR)/vmlinux.bcm $(KDIR)/vmlinux.lzma
	$(STAGING_DIR_HOST)/bin/imagetag_nand -K -i $(KDIR)/vmlinux.lzma -o $(KDIR)/vmlinux.lz -e $(KERNEL_ENTRY)

	dd if=/dev/zero of=$(TMP_DIR)/file128k.tmp bs=1k count=128

	echo -e "/cferam.000" > $(KDIR)/nocomprlist
	echo -e "/vmlinux.lz" >> $(KDIR)/nocomprlist
	touch $(TARGET_DIR)/cferam.000
endef

define Image/Build
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	# Various routers
	$(call Image/Build/REF,$(1),963268,63268,VG50)
endef

$(eval $(call BuildImage))
