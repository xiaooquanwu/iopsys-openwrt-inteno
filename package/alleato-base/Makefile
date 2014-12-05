include $(TOPDIR)/rules.mk

PKG_NAME:=alleato-base

PKG_VERSION:=14.49.2
PKG_RELEASE=$(PKG_SOURCE_VERSION)
PKG_SOURCE_URL:=git@iopsys.inteno.se:alleato_base.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=8314aed451255410a2cd4fa43ac3235504e8f018

PKG_MAINTAINER:=Martin Assarsson, ALLEATO

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

######################################################

define Package/alleato3
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+python
  TITLE:=Alleato python libraries
endef

define Package/zwaved
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+jansson
  TITLE:=Alleato zwave daemon
endef

define Package/zwave-mgr
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+alleato3
  TITLE:=Alleato zwave manager
endef

define Package/zwave-profiler
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+alleato3
  TITLE:=Alleato zwave profiler
endef

define Package/alleato-scheduler
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+alleato3
  TITLE:=Alleato scheduler
endef

define Package/alleato-boxconfig
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+alleato3
  TITLE:=Alleato boxconfig
endef

define Package/all-omc
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=+alleato3 +screen
  TITLE:=Alleato remote tunnel
endef

define Package/ubus-kicker
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=
  TITLE:=Alleato ubus-kicker
endef

define Package/mqttnagios
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=
  TITLE:=Alleato mqttnagios
endef

define Package/all4-alleato-tools
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Alleato
  DEPENDS:=
  TITLE:=Alleato tools
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
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d $(1)/etc/alleato/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/zwave-mgr $(1)/etc/init.d/zwave-mgr
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/zwave_mgr $(1)/usr/mqttrules/zwave_mgr
endef

define Package/zwave-profiler/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/zwave-profiler $(1)/etc/init.d/zwave-profiler
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/zwave-profiler $(1)/usr/mqttrules/zwave-profiler
endef

define Package/alleato-scheduler/install
	$(INSTALL_DIR) $(1)/usr/mqttrules $(1)/etc/init.d $(1)/etc/crontabs/sched
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/scheduler $(1)/etc/init.d/scheduler 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/scheduler $(1)/usr/mqttrules/scheduler
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/schedule $(1)/etc/crontabs/sched/schedule
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/schedule $(1)/etc/crontabs/schedule
endef

define Package/alleato-boxconfig/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/bin $(1)/etc/init.d $(1)/etc/alleato $(1)/home/mosquitto
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/boxconfig $(1)/etc/init.d/boxconfig
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/boxconfig $(1)/usr/bin/boxconfig
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/genmosquittoconf $(1)/usr/bin/genmosquittoconf
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/check_client_id $(1)/usr/sbin/check_client_id
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/boxconfig.conf $(1)/etc/boxconfig.conf
endef

define Package/all-omc/install
	$(INSTALL_DIR) $(1)/root $(1)/etc/init.d $(1)/etc/config $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/omc $(1)/etc/init.d/omc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/root/ssh_key $(1)/root/ssh_key
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/alleato.config $(1)/etc/config/alleato
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/omc $(1)/usr/bin/omc
endef

define Package/ubus-kicker/install
	$(INSTALL_DIR) $(1)/etc/init.d $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/ubus-kicker $(1)/etc/init.d/ubus-kicker
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/ubus-kicker $(1)/usr/sbin/ubus-kicker
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/akick $(1)/usr/sbin/akick
endef

define Package/mqttnagios/install
	$(INSTALL_DIR) $(1)/etc/nagios.d $(1)/usr/bin $(1)/etc/crontabs $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/mqttnagios $(1)/etc/init.d/mqttnagios
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/send_mqtt_nag $(1)/usr/bin/send_mqtt_nag
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nagios.d/dropbear $(1)/etc/nagios.d/dropbear.nag
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nagios.d/mosquitto $(1)/etc/nagios.d/mosquitto.nag
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nagios.d/ssh $(1)/etc/nagios.d/ssh.nag
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/nagios.d/zwaved $(1)/etc/nagios.d/zwaved.nag
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/data/mqttnagios $(1)/etc/crontabs/mqttnagios
	echo '/5 * * * * /usr/sbin/check_client_id' >> $(1)/etc/crontabs/root
endef

define Package/all4-alleato-tools/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/opt/alleato
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/house $(1)/etc/init.d/house
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/rulesd $(1)/etc/init.d/rulesd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/binaries/alleato_tools $(1)/usr/bin/alleato_tools
endef

######################################################

$(eval $(call BuildPackage,alleato3))
$(eval $(call BuildPackage,zwaved))
$(eval $(call BuildPackage,zwave-mgr))
$(eval $(call BuildPackage,zwave-profiler))
$(eval $(call BuildPackage,alleato-scheduler))
$(eval $(call BuildPackage,alleato-boxconfig))
$(eval $(call BuildPackage,all-omc))
$(eval $(call BuildPackage,ubus-kicker))
$(eval $(call BuildPackage,mqttnagios))
$(eval $(call BuildPackage,all4-alleato-tools))
