include $(TOPDIR)/rules.mk

PKG_NAME:=customer-field-vox25
PKG_VERSION:=1.0.0
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Package/customer-field-vox25
	HIDDEN:=1
  TITLE:=Inteno Field
  DEPENDS:=+routermodel-vox25
endef

define Package/customer-field-vox25/config

endef

define Package/customer-field-vox25/description
	Inteno Default package
endef

define Package/customer-field-vox25/install
	$(CP) $(PKG_BUILD_DIR)/fs/* $(1)/
endef

define Package/customer-field-vox25/postinst

endef

#$(eval $(call BuildPackage,customer-field-vox25))
