#!/bin/bash

cd "$(dirname "$0")"
set -e 

function create_routermodel_makefile(){
	local ROUTERMODEL="$1"
	local MAKEFILE="$2"
	local PACKAGE_NAME="routermodel-$ROUTERMODEL"
	local MAKEFILE_TPL=$(cat <<EOF
include \$(TOPDIR)/rules.mk

PKG_NAME:=${PACKAGE_NAME}
PKG_VERSION:=1.0.0
PKG_MAINTAINER:=Martin K. Schroder <mkschreder.uk@gmail.com>

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1

include \$(INCLUDE_DIR)/package.mk

define Build/Prepare
	
endef

define Build/Compile

endef

define Package/${PACKAGE_NAME}
	TITLE:=${PACKAGE_NAME}
	DEPENDS:=$(cat depends |grep -E "^$ROUTERMODEL\:.*" | cut -d ':' -f2)
endef

define Package/${PACKAGE_NAME}/config

endef

define Package/${PACKAGE_NAME}/description
	Router model support package
endef

define Package/${PACKAGE_NAME}/install
	\$(CP) ./fs/* \$(1)/
endef

define Package/${PACKAGE_NAME}/postinst

endef

\$(eval \$(call BuildPackage,${PACKAGE_NAME}))
EOF
) 
	echo "$MAKEFILE_TPL" > $MAKEFILE
}

for ROUTERMODEL in `ls -d */|sed 's/\([^\/]*\)\/$/\1/gi'`; do
	mkdir -p "$ROUTERMODEL/fs"
	echo "$ROUTERMODEL"; 
	$(create_routermodel_makefile $ROUTERMODEL "$ROUTERMODEL/Makefile"); 
done
