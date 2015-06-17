include $(TOPDIR)/rules.mk

PKG_NAME:=profile-quarantine-files
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

define Package/profile-quarantine-files
	TITLE:=profile-quarantine-files
	DEPENDS:=+profile-quarantine +routermodel-files 
endef

define Package/profile-quarantine-files/config

endef

define Package/profile-quarantine-files/description
	
endef

define Package/profile-quarantine-files/install
	
endef

define Package/profile-quarantine-files/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-quarantine-files))
