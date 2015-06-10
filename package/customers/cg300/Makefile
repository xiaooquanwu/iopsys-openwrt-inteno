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
	if PACKAGE_inteno-cg300
		choice 
			prompt "Customer Config"
			default PACKAGE_customer-cg300_field
			config PACKAGE_customer-cg300_anyfi
				bool "Anyfi"
			config PACKAGE_customer-cg300_field
				bool "Inteno Default"
			config PACKAGE_customer-cg300_blc
				bool "Blc"
			config PACKAGE_customer-cg300_console
				bool "Console"
			config PACKAGE_customer-cg300_elements
				bool "Elements"
			config PACKAGE_customer-cg300_get
				bool "Get"
			config PACKAGE_customer-cg300_lounea
				bool "Lounea"
			config PACKAGE_customer-cg300_netmedia
				bool "Netmedia"
			config PACKAGE_customer-cg300_nordix
				bool "Nordix"
			config PACKAGE_customer-cg300_teliasafety
				bool "Telia Safety"
		endchoice
		if PACKAGE_customer-cg300_anyfi
			config TARGET_CUSTOMER
				string
				default "ANYFI"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_field
			config TARGET_CUSTOMER
				string
				default "FIELD"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_blc
			config TARGET_CUSTOMER
				string
				default "BLC"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_console
			config TARGET_CUSTOMER
				string
				default "CONSOLE"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_elements
			config TARGET_CUSTOMER
				string
				default "ELEMENTS"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_get
			config TARGET_CUSTOMER
				string
				default "GET"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_lounea
			config TARGET_CUSTOMER
				string
				default "LOUNEA"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_netmedia
			config TARGET_CUSTOMER
				string
				default "NETMEDIA"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_nordix
			config TARGET_CUSTOMER
				string
				default "NORDIX"
				prompt "Customer ID"
		endif
		if PACKAGE_customer-cg300_teliasafety
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
