include $(TOPDIR)/rules.mk

PKG_NAME:=customer-vox25
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/customer-vox25
	CATEGORY:=Configs
	HIDDEN:=1
	TITLE:=VOX25 Customer Base Package
endef

define Package/customer-vox25/config
	if PACKAGE_routermodel-vox25
		choice 
			prompt "VOX25 Customer Config"
			default PACKAGE_customer-vodafone-vox25
			config PACKAGE_customer-vodafone-vox25
				bool "VOX25 Vodafone"
			config PACKAGE_customer-field-vox25
				bool "VOX25 Default"
		endchoice
		
		if PACKAGE_customer-vodafone-vox25
			config TARGET_CUSTOMER
				string
				default "VDF"
		endif
		if PACKAGE_customer-field-vox25
			config TARGET_CUSTOMER
				string
				default "INTFIELD"
		endif
	endif
	
endef

define Package/customer-vox25/description
	VOX25 base package
endef


$(eval $(call BuildPackage,customer-vox25))
