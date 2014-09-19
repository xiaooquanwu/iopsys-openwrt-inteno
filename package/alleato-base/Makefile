include $(TOPDIR)/rules.mk

PKG_NAME:=alleato-base

PKG_VERSION:=14.10.19
PKG_RELEASE=$(PKG_SOURCE_VERSION)
ifeq ($(CONFIG_PACKAGE_bcmkernel),y)
PKG_SOURCE_URL:=git@iopsys.inteno.se:alleato_base.git
else
PKG_SOURCE_URL:=http://ihgsp.inteno.se/git/alleato_base.git
endif
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=a7736dc4e6c32057fb138df9dcd9eaf01a271d3c

PKG_MAINTAINER:=Martin Assarsson, ALLEATO

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

######################################################

define Package/alleato3
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+python
  TITLE:=Alleato python libraries
endef

define Package/zwaved
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+jansson
  TITLE:=Alleato zwave daemon
endef

define Package/zwave-mgr
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato zwave manager
endef

define Package/zwave-profiler
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato zwave profiler
endef

define Package/alleato-house
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato house
endef

define Package/social-alarm
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato social alarm
endef

define Package/alleato-scheduler
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato scheduler
endef

define Package/alleato-boxconfig
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=alleato3
  TITLE:=Alleato boxconfig
endef

define Package/all-omc
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+alleato3 +screen
  TITLE:=Alleato remote tunnel
endef

define Package/bik
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+alleato3
  TITLE:=BIC generator
endef


######################################################

define Package/alleato3/install
	$(INSTALL_DIR) $(1)/usr/lib/python2.7/
	$(CP) -r $(PKG_BUILD_DIR)/python-2.7/* $(1)/usr/lib/python2.7/
endef

define Package/zwaved/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/zwaved $(1)/etc/init.d/zwaved
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/zwaved $(1)/usr/sbin/zwaved
endef

define Package/zwave-mgr/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/zwave-mgr $(1)/etc/init.d/zwave-mgr
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/zwave_mgr $(1)/usr/mqttrules/zwave_mgr
endef

define Package/zwave-profiler/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/zwave-profiler $(1)/etc/init.d/zwave-profiler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/zwave-profiler $(1)/usr/mqttrules/zwave-profiler
endef

define Package/alleato-house/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/house $(1)/etc/init.d/house
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/house $(1)/usr/mqttrules/house
endef

define Package/social-alarm/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/social-alarm $(1)/etc/init.d/social-alarm
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/etc/social-alarm.conf.example $(1)/etc/social-alarm.conf.example
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/social-alarm $(1)/usr/mqttrules/social-alarm
endef

define Package/alleato-scheduler/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/scheduler $(1)/etc/init.d/scheduler 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/scheduler $(1)/usr/mqttrules/scheduler
endef

define Package/alleato-boxconfig/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/bin $(1)/etc/init.d $(1)/etc/alleato
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/boxconfig $(1)/etc/init.d/boxconfig
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/boxconfig $(1)/usr/bin/boxconfig
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/genmosquittoconf $(1)/usr/bin/genmosquittoconf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/check_client_id $(1)/usr/sbin/check_client_id
endef

define Package/all-omc/install
	$(INSTALL_DIR) $(1)/root $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/omc $(1)/etc/init.d/omc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/root/ssh_key $(1)/root/ssh_key
endef

define Package/bik/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/bik $(1)/etc/init.d/bik
endef


######################################################

$(eval $(call BuildPackage,alleato3))
$(eval $(call BuildPackage,zwaved))
$(eval $(call BuildPackage,zwave-mgr))
$(eval $(call BuildPackage,zwave-profiler))
$(eval $(call BuildPackage,alleato-house))
$(eval $(call BuildPackage,social-alarm))
$(eval $(call BuildPackage,alleato-scheduler))
$(eval $(call BuildPackage,alleato-boxconfig))
$(eval $(call BuildPackage,all-omc))
$(eval $(call BuildPackage,bik))

