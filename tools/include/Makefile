include $(TOPDIR)/rules.mk

PKG_NAME:=profile-tools-include
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

define Package/profile-tools-include
	TITLE:=profile-tools-include
	DEPENDS:=+profile-tools +routermodel-include 
endef

define Package/profile-tools-include/config

endef

define Package/profile-tools-include/description
	
endef

define Package/profile-tools-include/install
	
endef

define Package/profile-tools-include/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-tools-include))
