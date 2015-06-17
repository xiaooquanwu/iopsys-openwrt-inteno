include $(TOPDIR)/rules.mk

PKG_NAME:=profile-include-site
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

define Package/profile-include-site
	TITLE:=profile-include-site
	DEPENDS:=+profile-include +routermodel-site 
endef

define Package/profile-include-site/config

endef

define Package/profile-include-site/description
	
endef

define Package/profile-include-site/install
	
endef

define Package/profile-include-site/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-include-site))
