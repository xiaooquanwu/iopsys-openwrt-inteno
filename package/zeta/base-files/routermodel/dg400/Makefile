include $(TOPDIR)/rules.mk

PKG_NAME:=routermodel-dg400
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

define Package/routermodel-dg400
	TITLE:=routermodel-dg400
	DEPENDS:= +inteno-base +inteno-base-voice +inteno-base-dect
endef

define Package/routermodel-dg400/config

endef

define Package/routermodel-dg400/description
	Router model support package
endef

define Package/routermodel-dg400/install
	$(CP) ./fs/* $(1)/
endef

define Package/routermodel-dg400/postinst

endef

$(eval $(call BuildPackage,routermodel-dg400))
