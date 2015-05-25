/*
 * wifimngr -- wifi manager
 *
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * Author: sukru.senli@inteno.se
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */


#include "wifi.h"

extern "C"
{
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
}

static struct uci_package *uci_wireless;

//WirelessRadio WDev[MAX_WDEV];
//WirelessInterface WIfc[MAX_WIFC];

//void
//populateWirelessInterfaces()
//{
//	struct uci_element *e;
//	struct uci_section *s;
//	const char *device;
//	int idx = 0;
//	int vif;
//	int vif0 = 0;
//	int vif1 = 0;
//	char wif[8];

//	uci_foreach_element(&uci_wireless->sections, e) {
//		s = uci_to_section(e);
//		if (!strcmp(s->type, "wifi-iface")) {
//			device = ugets(s, "device");
//			WIfc[idx].radio = device;
//			if (!strcmp(device, "wl0")) {
//				vif = vif0;
//				vif0++;
//			} else {
//				vif = vif1;
//				vif1++;
//			}
//			if (vif > 0)
//				sprintf(wif, "%s.%d", device, vif);
//			else
//				strcpy(wif, device);
//			WIfc[idx].ifname = std::string(wif);
//			WIfc[idx].ssid = ugets(s, "ssid");
//			WIfc[idx].encryption = ugets(s, "encryption");
//			WIfc[idx].key = ugets(s, "key");
//			idx++;
//		}
//	}
//}

//void
//populateWirelessRadios()
//{
//	struct uci_element *e;
//	struct uci_section *s;
//	int idx = 0;

//	if(!(uci_wireless = init_package("wireless")))
//		return;

//	uci_foreach_element(&uci_wireless->sections, e) {
//		s = uci_to_section(e);
//		if (!strcmp(s->type, "wifi-device")) {
//			WDev[idx].name = s->e.name;
//			WDev[idx].bwCap = ugeti(s, "bandwidth");
//			WDev[idx].hwmode = ugets(s, "hwmode");
//			WDev[idx].channel = ugeti(s, "channel");
//			WDev[idx].autoChanTimer = ugeti(s, "scantimer");
//			idx++;
//		}
//	}
//}

Wireless WL;
WPS Wps;

string Wireless::getChannels(int freq) {
	string wl;
	if (freq == 5)
		wl = WL.radio5g;
	else
		wl = WL.radio2g;

	return strCmd("wlctl -i %s channels", wl.c_str());
}

void Wireless::setChannel(int freq, int ch) {
	string wl;

	char channel[3];
	snprintf(channel, 3, "%d", ch);

	if (freq == 5) {
		wl = WL.radio5g;
		WL.channel5g = ch;
	} else {
		wl = WL.radio2g;
		WL.channel2g = ch;
	}

	uciSet("wireless", wl.c_str(), "channel", channel);
	uciCommit("wireless");
	runCmd("wlctl -i %s down", wl.c_str());
	runCmd("wlctl -i %s channel %d", wl.c_str(), ch);
	runCmd("wlctl -i %s up", wl.c_str());
}

void Wireless::setSsid(string ssid) {
	struct uci_element *e;
	struct uci_section *s;

	if (WL.ssid == ssid)
		return;

	WL.ssid = ssid;

	uci_foreach_element(&uci_wireless->sections, e) {
		s = uci_to_section(e);
		if (!strcmp(s->type, "wifi-iface")) {
			uset(s, "ssid", ssid.c_str());
		}
	}
	ucommit(s);

	runCmd("sed -i \"s/_ssid=.*/_ssid=%s/g\" /etc/config/broadcom", ssid.c_str());
	runCmd("killall -9 nas; nas");
	runCmd("killall -SIGTERM wps_monitor; wps_monitor &");
}

void Wireless::setKey(string key) {
	struct uci_element *e;
	struct uci_section *s;

	if (WL.key == key)
		return;

	WL.key = key;

	uci_foreach_element(&uci_wireless->sections, e) {
		s = uci_to_section(e);
		if (!strcmp(s->type, "wifi-iface")) {
			uset(s, "key", key.c_str());
		}
	}
	ucommit(s);

	runCmd("sed -i \"s/_wpa_psk=.*/_wpa_psk=%s/g\" /etc/config/broadcom", key.c_str());
	runCmd("killall -9 nas; nas");
	runCmd("killall -SIGTERM wps_monitor; wps_monitor &");
}

void WPS::activate(int status) {
	if (status) {
		if (strCmd("pidof wps_monitor") == "") {
			runCmd("wps_monitor &");
			usleep(100000);
		}
		runCmd("killall -SIGUSR2 wps_monitor");
	} else
		runCmd("killall -SIGTERM wps_monitor");
}

void WPS::changeStatus() {
	struct uci_element *e;
	struct uci_section *s;

	char wps[2];
	snprintf(wps, 2, "%d", Wps.enable);

	uci_foreach_element(&uci_wireless->sections, e) {
		s = uci_to_section(e);
		if (!strcmp(s->type, "wifi-iface")) {
			uset(s, "wps_pbc", wps);
		}
	}
	ucommit(s);

	if (Wps.enable) {
		runCmd("sed -i \"s/wps_mode 'disabled'/wps_mode 'enabled'/g\" /etc/config/broadcom");
		if (strCmd("pidof wps_monitor") == "")
			runCmd("wps_monitor &");
	} else {
		runCmd("sed -i \"s/wps_mode 'enabled'/wps_mode 'disabled'/g\" /etc/config/broadcom");
		runCmd("killall -SIGTERM wps_monitor 2>/dev/null");
	}
}

void
populateWireless()
{
	struct uci_element *e;
	struct uci_section *s;
	string channel, chanspec;

	if(!(uci_wireless = init_package("wireless")))
		return;

	Wps.enable = 0;
	WL.channel2g = 0;
	WL.channel5g = -1;

	uci_foreach_element(&uci_wireless->sections, e) {
		s = uci_to_section(e);
		if (!strcmp(s->type, "wifi-device")) {
			if(!strcmp(ugets(s, "band"), "a")) {
				WL.radio5g = s->e.name;
				chanspec = ugets(s, "channel");
				channel = chanspec.substr(0, chanspec.find('/'));
				if (!strcmp(channel.c_str(), "auto"))
					WL.channel5g = 0;
				else
					WL.channel5g = ugeti(s, "channel");
			} else {
				WL.radio2g = s->e.name;
				chanspec = ugets(s, "channel");
				channel = chanspec.substr(0, chanspec.find('/'));
				if (channel == "auto")
					WL.channel2g = 0;
				else
					WL.channel2g = ugeti(s, "channel");
			}
		} else if (!strcmp(s->type, "wifi-iface")) {
			WL.ssid = ugets(s, "ssid");
			WL.encryption = ugets(s, "encryption");
			WL.key = ugets(s, "key");
			if (ugeti(s, "wps_pbc") == 1)
				Wps.enable = 1;
		}
	}
}
