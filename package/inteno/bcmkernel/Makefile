#
# Primary/common bcmkernel makefile for all versions of Broadcom SDKs
#

include $(TOPDIR)/rules.mk

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

# Include SDK version specific makefile based on config selection
ifeq ($(CONFIG_ARCH),)
include $(TOPDIR)/.config
endif

ifeq ($(CONFIG_BCM_OPEN),y)
PKG_NAME:=bcmopen
PKG_VERSION:=consumer
PKG_RELEASE:=1

TARGET_VERSION=$(shell echo $(CONFIG_TARGET_VERSION) | sed s/\"//g)
TARGET_IBOARDID=$(shell echo $(CONFIG_TARGET_IBOARDID) | sed s/\"//g)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).$(TARGET_VERSION).$(TARGET_IBOARDID).tar.gz
PKG_SOURCE_URL:=http://ihgsp.inteno.se/ihgsp/consumer/
endif

ifeq ($(CONFIG_BRCM_SDK_VER_414040),y)
include ./414040.mk

else ifeq ($(CONFIG_BRCM_SDK_VER_416021),y)
include ./416021.mk

else ifeq ($(CONFIG_BRCM_SDK_VER_416030),y)
include ./416030.mk

else
# Make 'make menuconfig' work even though no SDK version specified
PKG_VERSION:=none
include $(INCLUDE_DIR)/package.mk
endif

# hack to be able to compile just the userspace part.
# TOPDIR=$PWD  make -C package/bcmkernel userspace
userspace:
	$(call Build/Compile/Default, userspace)

$(eval $(call BuildPackage,bcmkernel))

