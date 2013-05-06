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

	# Add jffs2 eof marker
	echo -ne '\xde\xad\xc0\xde' >> $(KDIR)/root.$(1)

	# Tag the rootfs image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/root.$(1) \
		-o $(BIN_DIR)/IOPSYS-$(4).w \
		-b $(2) \
                -c $(3) \
		-q NAND128

	# Concat the cfe and rootfs together
	cat $(BIN_DIR)/IOPSYS-$(3)_cfe.w $(KDIR)/root.$(1) > $(KDIR)/cfe_fs_image_128

	# Tag the combined image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe_fs_image_128 \
		-o $(BIN_DIR)/IOPSYS-$(4)_cfe.w \
		-b $(2) \
                -c $(3) \
		-q NAND128	

endef

define Image/Build/CFE
	# Tag the cfe image
	$(STAGING_DIR_HOST)/bin/imagetag_nand -f $(KDIR)/cfe$(2)_nand.v \
		-o $(BIN_DIR)/IOPSYS-$(2)_cfe.w \
		-b $(1) \
                -c $(2) \
		-q NAND128
endef


define Image/Prepare
	$(STAGING_DIR_HOST)/bin/lzma e -d22 -lp2 -lc1 $(KDIR)/vmlinux.bcm $(KDIR)/vmlinux.lzma
	$(STAGING_DIR_HOST)/bin/imagetag_nand -K -i $(KDIR)/vmlinux.lzma -o $(KDIR)/vmlinux.lz -e $(KERNEL_ENTRY)

	echo -e "/cferam.000" > $(KDIR)/nocomprlist
	echo -e "/vmlinux.lz" >> $(KDIR)/nocomprlist
	touch $(TARGET_DIR)/cferam.000
	cp $(KDIR)/vmlinux.lz $(TARGET_DIR)/
endef

define Image/Build
	$(call Image/Build/CFE,963268,63268)
	$(call Image/Build/REF,$(1),963268,63268,VG50-WU7U)
	$(call Image/Build/REF,$(1),963268,63268,DG301-W7P2U)
endef

$(eval $(call BuildImage))
