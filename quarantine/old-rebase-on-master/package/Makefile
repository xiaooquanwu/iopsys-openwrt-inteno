include $(TOPDIR)/rules.mk

PKG_NAME:=profile-quarantine-package
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

define Package/profile-quarantine-package
	TITLE:=profile-quarantine-package
	DEPENDS:=+profile-quarantine +routermodel-package 
endef

define Package/profile-quarantine-package/config

endef

define Package/profile-quarantine-package/description
	
endef

define Package/profile-quarantine-package/install
	
endef

define Package/profile-quarantine-package/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-quarantine-package))
