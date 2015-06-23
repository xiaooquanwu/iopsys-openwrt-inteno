#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=brcm63xx_ref_nand_arm
BOARDNAME:=Broadcom BCM63xx_ref_nand_arm
FEATURES:=
LINUX_VERSION:=3.3.8
MAINTAINER:=Benjamin Larsson <benjamin.larsson@inteno.se>
CFLAGS:=-Os -pipe -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for Broadcom based xDSL/routers
	based on the Broadcom SDK for arm
endef

$(eval $(call BuildTarget))
