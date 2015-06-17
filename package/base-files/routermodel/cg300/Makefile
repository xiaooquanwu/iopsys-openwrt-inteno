include $(TOPDIR)/rules.mk

PKG_NAME:=routermodel-cg300
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

define Package/routermodel-cg300
	TITLE:=routermodel-cg300
	DEPENDS:= +inteno-base +inteno-base-voice +inteno-base-dect
endef

define Package/routermodel-cg300/config

endef

define Package/routermodel-cg300/description
	Router model support package
endef

define Package/routermodel-cg300/install
	
endef

define Package/routermodel-cg300/postinst

endef

$(eval $(call BuildPackage,routermodel-cg300))
