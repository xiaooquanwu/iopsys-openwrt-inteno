include $(TOPDIR)/rules.mk

PKG_NAME:=customer-cg300
PKG_VERSION:=15.6.8
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Build/Prepare

endef

define Package/customer-cg300
	CATEGORY:=Configs
  HIDDEN:=1
  TITLE:=CG300 Customer Base Configs
  DEPENDS:=
endef

define Package/customer-cg300/config
	if PACKAGE_routermodel-cg300
		choice 
			prompt "Customer Config"
			default PACKAGE_customer-field-cg300
			config PACKAGE_customer-cg300-anyfi
				bool "Anyfi"
			config PACKAGE_customer-field-cg300
				bool "Inteno Default"
			config PACKAGE_customer-blc-cg300
				bool "Blc"
			config PACKAGE_customer-console-cg300
				bool "Console"
			config PACKAGE_customer-elements-cg300
				bool "Elements"
			config PACKAGE_customer-get-cg300
				bool "Get"
			config PACKAGE_customer-lounea-cg300
				bool "Lounea"
			config PACKAGE_customer-netmedia-cg300
				bool "Netmedia"
			config PACKAGE_customer-nordix-cg300
				bool "Nordix"
			config PACKAGE_customer-teliasafety-cg300
				bool "Telia Safety"
		endchoice
		if PACKAGE_customer-cg300-anyfi
			config TARGET_CUSTOMER
				string
				default "ANYFI"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-field-cg300
			config TARGET_CUSTOMER
				string
				default "FIELD"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-blc-cg300
			config TARGET_CUSTOMER
				string
				default "BLC"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-console-cg300
			config TARGET_CUSTOMER
				string
				default "CONSOLE"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-elements-cg300
			config TARGET_CUSTOMER
				string
				default "ELEMENTS"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-get-cg300
			config TARGET_CUSTOMER
				string
				default "GET"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-lounea-cg300
			config TARGET_CUSTOMER
				string
				default "LOUNEA"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-netmedia-cg300
			config TARGET_CUSTOMER
				string
				default "NETMEDIA"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-nordix-cg300
			config TARGET_CUSTOMER
				string
				default "NORDIX"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-teliasafety-cg300
			config TARGET_CUSTOMER
				string
				default "TELIASAFETY"
				prompt "Customer ID"
		endif
	endif
endef

define Package/customer-cg300/description
	CG300 Customer Base Configuration
endef

define Package/customer-cg300/install
	
endef

define Package/customer-cg300/postinst

endef

$(eval $(call BuildPackage,customer-cg300))
