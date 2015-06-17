include $(TOPDIR)/rules.mk

PKG_NAME:=profile-package-devel
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

define Package/profile-package-devel
	TITLE:=profile-package-devel
	DEPENDS:=+profile-package +routermodel-devel 
endef

define Package/profile-package-devel/config

endef

define Package/profile-package-devel/description
	
endef

define Package/profile-package-devel/install
	
endef

define Package/profile-package-devel/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-package-devel))
