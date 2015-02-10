include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME    := wl-anyfi
PKG_VERSION := 6.37.14.4803.cpe4.14L04.0-anyfi1.3
PKG_RELEASE := 1

PKG_BUILD_DIR := $(BUILD_DIR)/wl-$(PKG_VERSION)

BCM_KERNEL = $(shell grep 'BCM_KERNEL_VERSION:=' $(TOPDIR)/package/bcmopen/Makefile | cut -d'=' -f2)

MODULES_DIR = "/lib/modules/$(BCM_KERNEL)/extra"

include $(INCLUDE_DIR)/package.mk

define Package/wl-anyfi
  SECTION  := kernel
  SUBMENU  := Proprietary BCM43xx Wi-Fi driver with Anyfi.net support
  CATEGORY := Kernel modules
  SUBMENU  := Wireless Drivers
  TITLE    := Proprietary BCM43xx Wi-Fi driver with Anyfi.net support
  DEPENDS  := +bcmkernel
endef

define Package/wl-anyfi/description
   Proprietary BCM43xx WiFi driver with Anyfi.net
   support, built from precompiled kernel object files.

   NOTE: This package is only needed until the Broadcom mainline
         driver can support Anyfi.net interfaces.
endef

define Build/Configure
	@echo "Nothing to do - wl-anyfi comes pre-built."
endef

define Build/Compile
	@echo "Nothing to do - wl-anyfi comes pre-built."
	tar xf files/wl-$(PKG_VERSION).tar.bz2 -C $(BUILD_DIR)
	cd $(PKG_BUILD_DIR) && ln -fs wl963268GW-anyfi.ko wl963268GWV-anyfi.ko
endef

define Package/wl-anyfi/preinst
#!/bin/sh
cd $${IPKG_INSTROOT}/$(MODULES_DIR) && mv wl.ko wl-bcm.ko
endef

define Package/wl-anyfi/postrm
#!/bin/sh
cd $${IPKG_INSTROOT}/$(MODULES_DIR) && mv wl-bcm.ko wl.ko
endef

define Package/wl-anyfi/install
	$(INSTALL_DIR) $(1)/$(MODULES_DIR)
	cp $(PKG_BUILD_DIR)/wl$(CONFIG_BCM_KERNEL_PROFILE)-anyfi.ko $(1)/$(MODULES_DIR)/wl.ko
endef

$(eval $(call BuildPackage,wl-anyfi))
