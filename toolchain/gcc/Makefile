include $(TOPDIR)/rules.mk

PKG_NAME:=profile-toolchain-gcc
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

define Package/profile-toolchain-gcc
	TITLE:=profile-toolchain-gcc
	DEPENDS:=+profile-toolchain +routermodel-gcc 
endef

define Package/profile-toolchain-gcc/config

endef

define Package/profile-toolchain-gcc/description
	
endef

define Package/profile-toolchain-gcc/install
	
endef

define Package/profile-toolchain-gcc/postinst
	$(CP) ./fs/* $(1)/
endef

$(eval $(call BuildPackage,profile-toolchain-gcc))
