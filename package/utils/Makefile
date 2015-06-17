include $(TOPDIR)/rules.mk

PKG_NAME:=profile-package-utils
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

define Package/profile-package-utils
	TITLE:=profile-package-utils
	DEPENDS:=+profile-package +routermodel-utils 
endef

define Package/profile-package-utils/config

endef

define Package/profile-package-utils/description
	
endef

define Package/profile-package-utils/install
	
endef

define Package/profile-package-utils/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-package-utils))
