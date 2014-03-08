include $(TOPDIR)/rules.mk

ANYFI_TARGET := $(ARCH)-linux-$(subst C,c,$(LIBC))-$(LIBCV)

PKG_NAME     := anyfi
PKG_VERSION  := 1.2.5
PKG_RELEASE  := 1

PKG_SOURCE     := anyfimac-$(PKG_VERSION)-$(ANYFI_TARGET).tar.bz2
PKG_SOURCE_URL := http://anyfi.net/download
PKG_BUILD_DIR  := $(BUILD_DIR)/anyfimac-$(PKG_VERSION)-$(ANYFI_TARGET)

include $(INCLUDE_DIR)/package.mk

define Package/anyfi
  SECTION  := net
  CATEGORY := Network
  TITLE    := Anyfi.net - The Open Wi-Fi Mobility Platform
  URL      := http://anyfi.net
  DEPENDS  := +kmod-tun +librt +wl-anyfi
endef

define Package/anyfi/description
	Anyfi.net binaries and integration scripts.
endef

define Package/anyfi/config
config ANYFI_CONTROLLER
	string "Controller IP or FQDN"
	default demo.anyfi.net
	depends PACKAGE_anyfi
	help
          The IP address or fully qualified domain name (FQDN) of the Controller to use.
endef

define Build/Compile
	@echo "Nothing to do - Anyfi.net software comes pre-built."
endef

define Package/anyfi/preinst
#!/bin/sh
cat <<EOF >$${IPKG_INSTROOT}/etc/config/anyfi
config anyfi config
	option controller $(CONFIG_ANYFI_CONTROLLER)
EOF
endef

define Package/anyfi/postrm
#!/bin/sh
rm -f $${IPKG_INSTROOT}/etc/config/anyfi
endef

define Package/anyfi/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/anyfimac $(1)/sbin
	$(LN) anyfimac $(1)/sbin/anyfid
	$(LN) anyfimac $(1)/sbin/myfid
	$(INSTALL_DIR) $(1)/bin
	$(LN) ../sbin/anyfimac $(1)/bin/anyfidctl
	$(LN) ../sbin/anyfimac $(1)/bin/myfidctl
	$(INSTALL_DIR) $(1)/lib/network
	$(INSTALL_BIN) ./files/anyfi.sh $(1)/lib/network
endef

$(eval $(call BuildPackage,anyfi))
