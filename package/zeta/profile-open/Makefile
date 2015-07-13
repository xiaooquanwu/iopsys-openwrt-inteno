include $(TOPDIR)/rules.mk

PKG_NAME:=profile-open
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

define Package/profile-open
	TITLE:=profile-open
	DEPENDS:= +inteno-base-juci +juci-theme-inteno
endef

define Package/profile-open/config

endef

define Package/profile-open/description
	Router model support package
endef

define Package/profile-open/install
	
endef

define Package/profile-open/postinst

endef

$(eval $(call BuildPackage,profile-open))
