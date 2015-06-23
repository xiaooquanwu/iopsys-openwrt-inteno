include $(TOPDIR)/rules.mk

PKG_NAME:=profile-quarantine-configs
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

define Package/profile-quarantine-configs
	TITLE:=profile-quarantine-configs
	DEPENDS:=+profile-quarantine +routermodel-configs 
endef

define Package/profile-quarantine-configs/config

endef

define Package/profile-quarantine-configs/description
	
endef

define Package/profile-quarantine-configs/install
	
endef

define Package/profile-quarantine-configs/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-quarantine-configs))
