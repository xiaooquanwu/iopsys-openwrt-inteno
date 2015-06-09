include $(TOPDIR)/rules.mk

PKG_NAME:=cg300_netmedia
PKG_VERSION:=1.0.0
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/cg300_netmedia
	HIDDEN:=1
  TITLE:=Inteno Field
  DEPENDS:=+cg300
endef

define Package/cg300_netmedia/config

endef

define Package/cg300_netmedia/description
	Inteno Default package
endef

define Package/cg300_netmedia/install
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
endef

define Package/cg300_netmedia/postinst

endef

#$(eval $(call BuildPackage,cg300_netmedia))
