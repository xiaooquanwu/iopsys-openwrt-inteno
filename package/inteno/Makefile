include $(TOPDIR)/rules.mk

PKG_NAME:=profile-package-inteno
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

define Package/profile-package-inteno
	TITLE:=profile-package-inteno
	DEPENDS:=+profile-package +routermodel-inteno 
endef

define Package/profile-package-inteno/config

endef

define Package/profile-package-inteno/description
	
endef

define Package/profile-package-inteno/install
	
endef

define Package/profile-package-inteno/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-package-inteno))
