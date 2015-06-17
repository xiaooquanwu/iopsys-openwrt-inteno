include $(TOPDIR)/rules.mk

PKG_NAME:=profile-package-boot
PKG_VERSION:=1.0.0
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Build/Compile

endef

define Package/profile-package-boot
	TITLE:=profile-package-boot
	DEPENDS:=+profile-package +routermodel-boot 
endef

define Package/profile-package-boot/config

endef

define Package/profile-package-boot/description
	
endef

define Package/profile-package-boot/install
	
endef

define Package/profile-package-boot/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-package-boot))
