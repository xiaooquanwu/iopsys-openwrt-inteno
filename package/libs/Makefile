include $(TOPDIR)/rules.mk

PKG_NAME:=profile-package-libs
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

define Package/profile-package-libs
	TITLE:=profile-package-libs
	DEPENDS:=+profile-package +routermodel-libs 
endef

define Package/profile-package-libs/config

endef

define Package/profile-package-libs/description
	
endef

define Package/profile-package-libs/install
	
endef

define Package/profile-package-libs/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-package-libs))
