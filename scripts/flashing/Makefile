include $(TOPDIR)/rules.mk

PKG_NAME:=profile-scripts-flashing
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

define Package/profile-scripts-flashing
	TITLE:=profile-scripts-flashing
	DEPENDS:=+profile-scripts +routermodel-flashing 
endef

define Package/profile-scripts-flashing/config

endef

define Package/profile-scripts-flashing/description
	
endef

define Package/profile-scripts-flashing/install
	
endef

define Package/profile-scripts-flashing/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-scripts-flashing))
