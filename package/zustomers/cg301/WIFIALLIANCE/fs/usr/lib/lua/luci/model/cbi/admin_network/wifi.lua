--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: wifi.lua 9558 2012-12-18 13:58:22Z jow $
]]--

local wa = require "luci.tools.webadmin"
local nw = require "luci.model.network"
local ut = require "luci.util"
local nt = require "luci.sys".net
local fs = require "nixio.fs"

arg[1] = arg[1] or ""

m = Map("wireless", "",
	translate("The <em>Device Configuration</em> section covers settings which are shared among all defined wireless networks (if the radio hardware is multi-SSID capable). " ..
		"Per network settings like encryption or operation mode are grouped in the <em>Interface Configuration</em>."))


local ifsection

function m.on_commit(map)
	local wnet = nw:get_wifinet(arg[1])
	if ifsection and wnet then
		ifsection.section = wnet.sid
		m.title = luci.util.pcdata(wnet:get_i18n())
	end
end

nw.init(m.uci)

local wnet = nw:get_wifinet(arg[1])
local wdev = wnet and wnet:get_device()

-- redirect to overview page if network does not exist anymore (e.g. after a revert)
if not wnet or not wdev then
	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
	return
end

-- wireless toggle was requested, commit and reload page
function m.parse(map)
	if m:formvalue("cbid.wireless.%s.__toggle" % wdev:name()) then
		local action
		if wdev:get("disabled") == "1" or wnet:get("disabled") == "1" then
			wnet:set("disabled", nil)
			action = "enable"
		else
			wnet:set("disabled", "1")
			action = "disable"
		end
		wdev:set("disabled", nil)

		nw:commit("wireless")
		luci.sys.call("env -i /sbin/wifi %s %s >/dev/null 2>/dev/null" %{action, wnet:ifname()})

		luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless", arg[1]))
		return
	elseif m:formvalue("cbid.wireless.%s.__autoch" % wdev:name()) then
		local acs_mode = tonumber(luci.sys.exec("acs_cli -i %s mode | cut -d':' -f1" %wdev:name())) or 0
		local chan = tonumber(luci.sys.exec("wlctl -i %q status | grep Primary | awk '{print$NF}'" %wdev:name())) or 1
		luci.sys.exec("acs_cli -i %s mode 2 >/dev/null 2>/dev/null" %wdev:name())
		if chan >= 52 then
			-- switch to a non-dfs channel before running autochannel
			luci.sys.exec("wlctl -i %q down 2>/dev/null" %wdev:name())
			luci.sys.exec("wlctl -i %q channel 44/80 2>/dev/null" %wdev:name())
			luci.sys.exec("wlctl -i %q up 2>/dev/null" %wdev:name())
		end
		luci.sys.exec("acs_cli -i %s autochannel >/dev/null 2>/dev/null" %wdev:name())
		luci.sys.exec("acs_cli -i %s mode %d >/dev/null 2>/dev/null" %{wdev:name(), acs_mode})
		luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless", arg[1]))
		return
	end
	Map.parse(map)
end

m.title = luci.util.pcdata(wnet:get_i18n())


s = m:section(NamedSection, wdev:name(), "wifi-device", translate("Device Configuration"))
s.addremove = false

s:tab("general", translate("General Setup"))
s:tab("macfilter", translate("MAC-Filter"))
s:tab("advanced", translate("Advanced Settings"))
if TECUSER then
s:tab("antenna", translate("Antenna Selection"))
end
if ADMINST then
s:tab("anyfi", "Anyfi.net")
end

wrn = s:taboption("general", DummyValue, "__warning")
wrn.template = "admin_network/wifi_warning"
wrn.ifname   = arg[1]

st = s:taboption("general", DummyValue, "__status", translate("Status"))
st.template = "admin_network/wifi_status"
st.ifname   = arg[1]

en = s:taboption("general", Button, "__toggle")

if wdev:get("disabled") == "1" or wnet:get("disabled") == "1" then
	en.title      = translate("%s is disabled" %(wnet:ssid() or wnet:ifname() or "Wireless network"))
	en.inputtitle = translate("Enable")
	en.inputstyle = "apply"
else
	en.title      = translate("%s is enabled" %(wnet:ssid() or wnet:ifname() or "Wireless network"))
	en.inputtitle = translate("Disable")
	en.inputstyle = "reset"
end


local hwtype = wdev:get("type")
local htcaps = wdev:get("ht_capab") and true or false

-- NanoFoo
local nsantenna = wdev:get("antenna")


local has_apsta = nil
local has_sta = nil
local _, dev, net
-- Check whether there is a client interface on this router
for _, dev in ipairs(nw:get_wifidevs()) do
	if dev:get("apsta") == "1" then
		has_apsta = dev
	end
end
-- Check whether there is a client interface on the same radio
for _, net in ipairs(wdev:get_wifinets()) do
	if net:mode() == "sta" and net:id() ~= wnet:id() then
		has_sta = net
		break
	end
end

--[[
if has_sta then
	-- lock the channel choice as the station will dicatate the freq
	ch = s:taboption("advanced", DummyValue, "choice", translate("Channel"))
	ch.value = translatef("Locked to channel %d used by %s",
		has_sta:channel(), has_sta:shortname())
end
]]

------------------- Broadcom Device ------------------

if hwtype == "broadcom" then

	country = s:taboption("general", ListValue, "country", translate("Country"))
	for code, cntry in ut.vspairs(wdev:countries()) do
		country:value(code, cntry)
	end

	maxassc = s:taboption("general", Value, "maxassoc", translate("Connection Limit"))
	maxassc.default = "16"

	function maxassc.validate(self, value, section)
		local nvalue = tonumber(value) or 16
		if nvalue < 1 or nvalue > 128 then
			return nil, "Connection Limit value must be within 1-128 range"
		end
		return value
	end

	band = s:taboption("advanced", ListValue, "band", translate("Band"))
	if wdev:is_2g() then
		band:value("b", translate("2.4GHz"))
	end
	if wdev:is_5g() then
		band:value("a", translate("5GHz"))
	end

	bw = s:taboption("advanced", ListValue, "bandwidth", translate("Bandwidth"))
	bw:value("20", "20MHz Only")
	bw:value("40", "20/40MHz")
	if wdev:hwmodes().ac then
		bw:value("80", "20/40/80MHz", {band="a", country="US"}, {band="a", country="EU/13"})
	end
	if wdev:is_2g() then
		bw.default = "20"
	else
		bw.default = "40"
	end

	mode = s:taboption("advanced", ListValue, "hwmode", translate("Mode"))
	mode:value("auto", "Auto", {bandwidth="20"}, {bandwidth="40"})
	mode:value("11a", "802.11a", {band="a", bandwidth="20"})
	mode:value("11b", "802.11b", {band="b", bandwidth="20"})
	mode:value("11bg", "802.11b+g", {band="b", bandwidth="20"})
	mode:value("11g", "802.11g", {band="b", bandwidth="20"})
	mode:value("11gst", "802.11g + Turbo", {band="b", bandwidth="20"})
	mode:value("11lrs", "802.11 LRS", {band="b", bandwidth="20"})
	if wdev:hwmodes().ac then 		
		mode:value("11ac", "802.11n/ac", {band="a", bandwidth="20"}, {band="a", bandwidth="40"}, {band="a", bandwidth="80"})
	else
		mode:value("11n", "802.11n", {band="b", bandwidth="20"}, {band="b", bandwidth="40"}, {band="a", bandwidth="20"}, {band="a", bandwidth="40"})
	end

	ch = s:taboption("advanced", ListValue, "channel", translate("Channel"))
	ch:value("auto", translate("Auto"))


	function set_channel_val_dep(chnspec)
		local channel
		if chnspec:match("/80") then
			channel = chnspec:sub(0, chnspec:find("/") - 1)
			ch:value(chnspec, channel, {band="a", bandwidth="80"})
		elseif chnspec:match("l") then
			channel = chnspec:sub(0, chnspec:find("l") - 1) .. " @lower sideband"
			ch:value(chnspec, channel, {bandwidth="40"})
		elseif chnspec:match("u") then
			channel = chnspec:sub(0, chnspec:find("u") - 1) .. " @upper sideband"
			ch:value(chnspec, channel, {bandwidth="40"})
		else
			channel = chnspec
			ch:value(chnspec, channel, {bandwidth="20"})
		end
	end

	for _, bwh in pairs({"20", "40", "80"}) do
		for chn in wdev:channels(country:formvalue(wdev:name()) or wdev:get("country"), band:formvalue(wdev:name()) or wdev:get("band"), bwh) do
			if chn ~= "" then
				set_channel_val_dep(chn)
			end
		end
	end

	if wdev:is_5g() then
		dfsc = s:taboption("advanced", Flag, "dfsc", translate("DFS Channel Selection"), translate("Note: Some wireless devices do not support DFS/TPC channels"))
		dfsc:depends("channel", "auto")
		dfsc.rmempty = true
	end

	if wdev:get("channel") == "auto" then
		ach = s:taboption("advanced", Button, "__autoch")
		ach:depends("channel", "auto")
		ach.title      = translate("Current channel is %s" %wnet:channel())
		ach.inputtitle = translate("Force Auto Channel Selection")
		ach.inputstyle = "apply"
	end

if TECUSER then
	timer = s:taboption("advanced", Value, "scantimer", translate("Auto Channel Timer"), "min")
	timer:depends("channel", "auto")
	timer.default = 15
	timer.rmempty = true

	rifs = s:taboption("advanced", ListValue, "rifs", translate("RIFS"))
	rifs:depends("hwmode", "auto")
	rifs:depends("hwmode", "11n")
	rifs:value("0", "Off")	
	rifs:value("1", "On")

	rifsad = s:taboption("advanced", ListValue, "rifs_advert", translate("RIFS Advertisement"))
	rifsad:depends("hwmode", "auto")
	rifsad:depends("hwmode", "11n")
	rifsad:value("0", "Off")	
	rifsad:value("-1", "Auto")

	obss = s:taboption("advanced", ListValue, "obss_coex", translate("OBSS Co-Existence"))
	obss:depends("bandwidth", "40")
	obss:depends("bandwidth", "80")
	obss:value("1", "Enable")
	obss:value("0", "Disable")
end

--	rate = s:taboption("advanced", ListValue, "rate", translate("Rate Limit"))
--	rate:value("auto", "No Limit")
--	rate:value("1", "1 Mbps", {band="b"})
--	rate:value("2", "2 Mbps", {band="b"})
--	rate:value("5.5", "5.5 Mbps", {band="b"})
--	rate:value("6", "6 Mbps")
--	rate:value("9", "9 Mbps")
--	rate:value("11", "11 Mbps", {band="b"})
--	rate:value("12", "12 Mbps")
--	rate:value("18", "18 Mbps")
--	rate:value("24", "24 Mbps")
--	rate:value("36", "36 Mbps")
--	rate:value("48", "48 Mbps")
--	rate:value("54", "54 Mbps")

--	rateset = s:taboption("advanced", Value, "rateset", translate("Basic Rate"))
--	rateset:value("default", "Default")
--	rateset:value("all", "All")

	rxcps = s:taboption("advanced", ListValue, "rxchainps", translate("RX Chain Power Save"))
	rxcps:value("0", "Disable")	
	rxcps:value("1", "Enable")

if TECUSER then
	rxcpsqt = s:taboption("advanced", Value, "rxchainps_qt", translate("RX Chain Power Save Quite Time"))
	rxcpsqt.default = 10
	rxcpspps = s:taboption("advanced", Value, "rxchainps_pps", translate("RX Chain Power Save PPS"))
	rxcpspps.default = 10

	s:taboption("advanced", Flag, "frameburst", translate("Frame Bursting"))

	--s:taboption("advanced", Value, "distance", translate("Distance Optimization"))
	--s:option(Value, "slottime", translate("Slot time"))

	frag = s:taboption("advanced", Value, "frag", translate("Fragmentation Threshold"))
	frag.default = 2346
	rts = s:taboption("advanced", Value, "rts", translate("RTS Threshold"))
	rts.default = 2347
	dtim = s:taboption("advanced", Value, "dtim_period", translate("DTIM Interval"))
	dtim.default = 1
	bcn = s:taboption("advanced", Value, "beacon_int", translate("Beacon Interval"))
	bcn.default = 100
	
--	sm = s:taboption("advanced", ListValue, "doth", "Regulatory Mode")
--	sm:value("0", "Off")
--	sm:value("1", "Loose interpretation of 11h spec")
--	sm:value("2", "Strict interpretation of 11h spec")
--	sm:value("3", "Disable 11h and enable 11d")
end

	pwr = s:taboption("advanced", ListValue, "txpower", translate("Transmit Power"))
	pwr:value("10", "10%")
	pwr:value("20", "20%")
	pwr:value("30", "30%")
	pwr:value("40", "40%")
	pwr:value("50", "50%")
	pwr:value("60", "60%")
	pwr:value("70", "70%")
	pwr:value("80", "80%")
	pwr:value("90", "90%")
	pwr:value("100", "100%")

	wm = s:taboption("advanced", ListValue, "wmm", translate("WMM Mode"))
	wm:value("-1", "Auto")	
	wm:value("1", "Enable")
	wm:value("0", "Disable")
	wn = s:taboption("advanced", ListValue, "wmm_noack", translate("WMM No Acknowledgement"))
	wn:depends({wmm="1"})
	wn:depends({wmm="-1"})
	wn:value("1", "Enable")
	wn:value("0", "Disable")
	wa = s:taboption("advanced", ListValue, "wmm_apsd", translate("WMM APSD"))
	wa:depends({wmm="1"})
	wa:depends({wmm="-1"})
	wa:value("1", "Enable")
	wa:value("0", "Disable")

--if TECUSER then
--	if wdev:antenna().txant then
--		ant1 = s:taboption("antenna", ListValue, "txantenna", translate("Transmitter Antenna"))
--		ant1.widget = "radio"
--		ant1:depends("diversity", "")
--		ant1:value("3", translate("auto"))
--		ant1:value("0", translate("Antenna 1"))
--		ant1:value("1", translate("Antenna 2"))
--		ant1.default = "3"
--	end

--	if wdev:antenna().rxant then
--		ant2 = s:taboption("antenna", ListValue, "rxantenna", translate("Receiver Antenna"))
--		ant2.widget = "radio"
--		ant2:depends("diversity", "")
--		ant2:value("3", translate("auto"))
--		ant2:value("0", translate("Antenna 1"))
--		ant2:value("1", translate("Antenna 2"))
--		ant2.default = "3"
--	end
--end
end -- hwtype == "broadcom" --

function anyfi_bandwidth_is_valid(value)
	if not tonumber(value) or tonumber(value) < 1 or tonumber(value) > 100 then
		return false
	else
		return true
	end
end

------------------- Anyfi.net global configuration ------------------
local anyfi_controller

if ADMINST and (fs.access("/sbin/anyfid") or fs.access("/sbin/myfid")) then

	anyfi_cntrl = s:taboption("anyfi", Value, "anyfi_controller", "Controller", translate("A Fully Qualified Domain Name or IP address (e.g. demo.anyfi.net)"))
	anyfi_cntrl.rmempty = true

	anyfi_cntrl.cfgvalue = function(self, section, value)
		return m.uci:get("anyfi", "controller", "hostname")
	end

	anyfi_cntrl.write = function(self, section, value)
		m.uci:set("anyfi", "controller", "hostname", value)
		m.uci:commit("anyfi")
	end

	anyfi_cntrl.remove = function(self, section)
		m.uci:delete("anyfi", "controller", "hostname")
		m.uci:commit("anyfi")
	end

	anyfi_controller = anyfi_cntrl:formvalue(wdev:name())
end

anyfi_controller = (not anyfi_controller) and m.uci:get("anyfi", "controller", "hostname")

------------------- Anyfi.net device configuration ------------------

if ADMINST and os.execute("/sbin/anyfi-probe " .. hwtype .. " >/dev/null") == 0 and anyfi_controller and anyfi_controller ~= "" then
	anyfi_floor = s:taboption("anyfi", Value, "anyfi_floor", "Floor",
				  translate("The percentage of available spectrum and backhaul that mobile users are allowed to consume even if there is competition with the primary user"))

	anyfi_floor.rmempty = true
	anyfi_floor.default = "5"
	anyfi_floor:value("10", "10%")
	anyfi_floor:value("20", "20%")
	anyfi_floor:value("30", "30%")
	anyfi_floor:value("40", "40%")
	anyfi_floor:value("50", "50%")
	anyfi_floor:value("60", "60%")
	anyfi_floor:value("70", "70%")
	anyfi_floor:value("80", "80%")
	anyfi_floor:value("90", "90%")
	anyfi_floor:value("100", "100%")

	function anyfi_floor.validate(self, value, section)
		if anyfi_bandwidth_is_valid(value) then
			return value
		else
			return nil, "Invalid value for Floor, enter a value between 1-100 without '%'"
		end
	end

	anyfi_ceiling = s:taboption("anyfi", Value, "anyfi_ceiling", "Ceiling", translate("The maximum percentage of available spectrum and backhaul that mobile users are allowed to consume"))
	anyfi_ceiling.rmempty = true
	anyfi_ceiling.default = "75"
	anyfi_ceiling:value("10", "10%")
	anyfi_ceiling:value("20", "20%")
	anyfi_ceiling:value("30", "30%")
	anyfi_ceiling:value("40", "40%")
	anyfi_ceiling:value("50", "50%")
	anyfi_ceiling:value("60", "60%")
	anyfi_ceiling:value("70", "70%")
	anyfi_ceiling:value("80", "80%")
	anyfi_ceiling:value("90", "90%")
	anyfi_ceiling:value("100", "100%")

	function anyfi_ceiling.validate(self, value, section)
		if anyfi_bandwidth_is_valid(value) then
			return value
		else
			return nil, "Invalid value for Ceiling, enter a value between 1-100 without '%'"
		end
	end
end

----------------------- Interface -----------------------

s = m:section(NamedSection, wnet.sid, "wifi-iface", translate("Interface Configuration"))
ifsection = s
s.addremove = false
s.anonymous = true
s.defaults.device = wdev:name()

s:tab("general", translate("General Setup"))
s:tab("encryption", translate("Wireless Security"))
s:tab("macfilter", translate("MAC-Filter"))
s:tab("advanced", translate("Advanced Settings"))
s:tab("anyfi", translate("Anyfi.net"))

ssid = s:taboption("general", Value, "ssid", translate("<abbr title=\"Extended Service Set Identifier\">ESSID</abbr>"))

mode = s:taboption("general", ListValue, "mode", translate("Mode"))
mode.override_values = true
mode:value("ap", translate("Access Point"))
--if wdev:is_sta_capable() then
--mode:value("sta", translate("Client"))
--end


local network_msg = (TECUSER) and " or fill out the <em>create</em> field to define a new network." or "."
network = s:taboption("general", Value, "network", translate("Network"),
	translate("Choose the network(s) you want to attach to this wireless interface%s" %network_msg))

network.rmempty = true
network.template = "cbi/network_netlist"
network.widget = "checkbox"
network.novirtual = true

function network.write(self, section, value)
	m:chain("network")
	m:chain("firewall")
	local i = nw:get_interface(section)
	if i then
		if value == '-' then
			value = m:formvalue(self:cbid(section) .. ".newnet")
			if value and #value > 0 then
				local n = nw:add_network(value, {proto="none"})
				if n then n:add_interface(i) end
			else
				local n = i:get_network()
				if n then n:del_interface(i) end
			end
		else
			local v
			for _, v in ipairs(i:get_networks()) do
				v:del_interface(i)
			end
			for v in ut.imatch(value) do
				local n = nw:get_network(v)
				if n then
					if not n:is_empty() then
						n:set("type", "bridge")
					end
					n:add_interface(i)
				end
			end
		end
	end
end


-------------------- Broadcom Interface ----------------------

if hwtype == "broadcom" then
	function mode.write(self, section, value)
		if value == "sta" then
			wdev:set("apsta", "1")
			-- set autoconf for the ap interfaces on the same radio
			for _, net in ipairs(wdev:get_wifinets()) do
				if net:mode() == "ap" then
					net:set("autoconf", "1")
				end
			end
		else
			wdev:set("apsta", "0")
		end
		self.map:set(section, "mode", value)
	end

	if has_apsta then
		autoconf = s:taboption("general", Flag, "autoconf", translate("Auto-Configure"), translate("Overwrite SSID and security settings of this interface" ..
		" with the credentials recieved from another wireless AP via WPS"))
		autoconf:depends({mode="ap"})
		autoconf.rmempty = true
		if has_sta then
			autoconf.default = "1"
		end
	end

	hidden = s:taboption("general", Flag, "hidden", translate("Hide <abbr title=\"Extended Service Set Identifier\">ESSID</abbr>"))
	hidden:depends({mode="ap"})

	bssmax = s:taboption("general", Value, "bss_max", translate("Maximum Client"))
	bssmax:depends("mode", "ap")
	bssmax.default = "16"

	function bssmax.validate(self, value, section)
		local maxassoc = tonumber(maxassc:formvalue(wdev:name())) or 16
		local nvalue = tonumber(value) or 16
		if nvalue < 1 or nvalue > 128 then
			return nil, "Maximum Client value must be within 1-128 range"
		elseif nvalue > maxassoc then
			return nil, "Maximum Client value cannot be higher than Connection Limit value (%d) of %s" %{maxassoc, wdev:name()}
		end
		return value
	end

	function bssmax.remove(self, section)
		local maxassoc = tonumber(maxassc:formvalue(wdev:name())) or 16
		local nvalue = (maxassoc < 16) and maxassoc or 16
		self.map:set(section, "bss_max", tostring(nvalue))
	end

	isolate = s:taboption("advanced", Flag, "isolate", translate("Separate Clients"), translate("Prevents client-to-client communication"))
	isolate:depends({mode="ap"})

	s:taboption("advanced", Flag, "wmf_bss_enable", "Enable Wireless Multicast Forwarding (WMF)")
	s:taboption("advanced", Flag, "wmm_bss_disable", translate("Disable WMM Advertise"))

	mf = s:taboption("macfilter", ListValue, "macfilter", translate("MAC-Address Filter"))
	mf:depends("mode", "ap")
	mf:value("0", translate("Disable"))
	mf:value("2", translate("Allow listed only"))
	mf:value("1", translate("Allow all except listed"))

	ml = s:taboption("macfilter", DynamicList, "maclist", translate("MAC-List"))
	ml:depends({macfilter="2"})
	ml:depends({macfilter="1"})
	nt.mac_clients(function(mac, name) ml:value(mac, "%s (%s)" %{ mac, name }) end)
end


------------------- WiFI-Encryption -------------------

wps = s:taboption("encryption", Flag, "wps_pbc", translate("WPS Available"))
if wdev:is_5g() then
wps:depends({encryption="none", mode="ap"})
wps:depends({encryption="psk2", mode="ap"})
wps:depends({encryption="mixed-psk", mode="ap"})
else
wps:depends({encryption="none"})
wps:depends({encryption="psk2"})
wps:depends({encryption="mixed-psk"})
end

function wps.write(self, section, value)
	local val = tonumber(value)
	local mode = tostring(mode:formvalue(section)) or wnet:get("mode")
	if val == 1 then
		if mode == "sta" then
			self.map.uci:foreach("wireless", "wifi-iface",
				function(s)
					self.map.uci:set("wireless", s['.name'], "wps_pbc", "0")
				end)
		else
			self.map.uci:foreach("wireless", "wifi-iface",
				function(s)
					if s.mode == "sta" then
						self.map.uci:set("wireless", s['.name'], "wps_pbc", "0")
					end
				end)
		end
	end
	self.map:set(section, "wps_pbc", value)
end

encr = s:taboption("encryption", ListValue, "encryption", translate("Encryption"))
encr.override_values = true
encr.override_depends = true
encr:value("none", "No Encryption")
encr:value("wep-open",   translate("WEP Open System"), {mode="ap"}, {mode="sta"})
encr:value("wep-shared", translate("WEP Shared Key"),  {mode="ap"}, {mode="sta"})
--encr:value("psk", "WPA-PSK", {mode="ap"}, {mode="sta"})
encr:value("psk2", "WPA2-PSK", {mode="ap"}, {mode="sta"})
encr:value("mixed-psk", "WPA/WPA2-PSK Mixed", {mode="ap"})
--encr:value("wpa", "WPA-EAP", {mode="ap"})
encr:value("wpa2", "WPA2-EAP", {mode="ap"})
encr:value("mixed-wpa", "WPA/WPA2-EAP Mixed", {mode="ap"})

function encr.cfgvalue(self, section)
	local v = tostring(ListValue.cfgvalue(self, section))
	if v == "wep" then
		return "wep-open"
	elseif v == "pskmixedpsk2" then
		return "mixed-psk"
	elseif v == "wpamixedwpa2" then
		return "mixed-wpa"
	elseif v and v:match("%+") then
		return (v:gsub("%+.+$", ""))
	end
	return v
end

encr.write = function(self, section, value)
	if not value then
		return nil
	end
	self.map.uci:set("wireless", section, "encryption", value)
	if value:match("none") or value:match("wpa") then
		self.map.uci:delete("wireless", section, "key")
	end
	if not value:match("wep") then
		self.map.uci:delete("wireless", section, "key1")
		self.map.uci:delete("wireless", section, "key2")
		self.map.uci:delete("wireless", section, "key3")
		self.map.uci:delete("wireless", section, "key4")
	end
end

wrn = s:taboption("encryption", DummyValue, "__warning", translate(" "), translate("Are you sure? If you disable Wireless Security, you will be vulnerable to attacks!"))
wrn:depends({encryption="none"})

cipher = s:taboption("encryption", ListValue, "cipher", translate("Cipher"))
--cipher:depends({encryption="psk"})
cipher:depends({encryption="psk2"})
cipher:depends({encryption="mixed-psk"})
cipher:value("auto", translate("auto"), {encryption="mixed-psk"})
cipher:value("ccmp", translate("Force CCMP (AES)"), {encryption="psk2"}, {encryption="mixed-psk"})
--cipher:value("tkip", translate("Force TKIP"))
cipher:value("tkip+ccmp", translate("Force TKIP and CCMP (AES)"), {encryption="mixed-psk"})

radius_server = s:taboption("encryption", Value, "radius_server", translate("Radius-Authentication-Server"))
radius_server:depends({mode="ap", encryption="wpa"})
radius_server:depends({mode="ap", encryption="wpa2"})
radius_server:depends({mode="ap", encryption="mixed-wpa"})
radius_server.rmempty = true
radius_server.datatype = "host"

radius_port = s:taboption("encryption", Value, "radius_port", translate("Radius-Authentication-Port"))
radius_port:depends({mode="ap", encryption="wpa"})
radius_port:depends({mode="ap", encryption="wpa2"})
radius_port:depends({mode="ap", encryption="mixed-wpa"})
radius_port.rmempty = true
radius_port.datatype = "port"
radius_port.default = 1812

radius_secret = s:taboption("encryption", Value, "radius_secret", translate("Radius-Authentication-Secret"))
radius_secret:depends({mode="ap", encryption="wpa"})
radius_secret:depends({mode="ap", encryption="wpa2"})
radius_secret:depends({mode="ap", encryption="mixed-wpa"})
radius_secret.rmempty = true
radius_secret.password = true

function default_wpa_key()
	local fd = io.open("/proc/nvram/WpaKey")
	if fd then
		local wpa_key = fd:read("*l")
		fd:close()
		if #wpa_key >= 8 then
			return wpa_key
		end
	end
	return "1234567890"
end

wpakey = s:taboption("encryption", Value, "_wpa_key", translate("Key"))
wpakey:depends("encryption", "psk")
wpakey:depends("encryption", "psk2")
wpakey:depends("encryption", "mixed-psk")
wpakey.datatype = "wpakey"
wpakey.rmempty = true
wpakey.password = true
wpakey.default = default_wpa_key()

net_reauth = s:taboption("encryption", Value, "net_reauth", translate("Network Re-auth Interval"))
net_reauth:depends({encryption="wpa"})
net_reauth:depends({encryption="wpa2"})
net_reauth:depends({encryption="mixed-wpa"})
net_reauth.default = 36000

gtk = s:taboption("encryption", Value, "gtk_rekey", translate("WPA Group Rekey Interval"))
gtk:depends({encryption="psk"})
gtk:depends({encryption="psk2"})
gtk:depends({encryption="mixed-psk"})
gtk.default = 0

wpakey.cfgvalue = function(self, section, value)
	local key = m.uci:get("wireless", section, "key")
	if key == "1" or key == "2" or key == "3" or key == "4" then
		return nil
	end
	return key
end

wpakey.write = function(self, section, value)
	self.map.uci:set("wireless", section, "key", value)
	self.map.uci:delete("wireless", section, "key1")
	self.map.uci:delete("wireless", section, "key2")
	self.map.uci:delete("wireless", section, "key3")
	self.map.uci:delete("wireless", section, "key4")
end

wpakey.remove = function(self, section, value)
	local enc = self.map.uci:get("wireless", section, "encryption")
	if enc and enc:match("psk") then
		local oldkey = self.map.uci:get("wireless", section, "key")
		if oldkey and #oldkey >= 8 then
			self.map.uci:set("wireless", section, "key", oldkey)
		else
			self.map.uci:set("wireless", section, "key", default_wpa_key())
		end
	end
	return nil
end


wepslot = s:taboption("encryption", ListValue, "_wep_key", translate("Used Key Slot"))
wepslot:depends("encryption", "wep-open")
wepslot:depends("encryption", "wep-shared")
wepslot:value("1", translatef("Key #%d", 1))
wepslot:value("2", translatef("Key #%d", 2))
wepslot:value("3", translatef("Key #%d", 3))
wepslot:value("4", translatef("Key #%d", 4))

wepslot.cfgvalue = function(self, section)
	local slot = tonumber(m.uci:get("wireless", section, "key"))
	if not slot or slot < 1 or slot > 4 then
		return 1
	end
	return slot
end

wepslot.write = function(self, section, value)
	self.map.uci:set("wireless", section, "key", value)
end

function default_wep_key(slot)
	local wep_key = ""
	for wep=1,10 do
		wep_key = wep_key .. "%d" %slot
	end
	return wep_key
end

local slot
for slot=1,4 do
	wepkey = s:taboption("encryption", Value, "key" .. slot, translatef("Key #%d", slot))
	wepkey:depends("encryption", "wep-open")
	wepkey:depends("encryption", "wep-shared")
	wepkey.datatype = "wepkey"
	wepkey.rmempty = true
	wepkey.password = true
	wepkey.default = default_wep_key(slot)

	function wepkey.remove(self, section, value)
		local enc = self.map.uci:get("wireless", section, "encryption")
		local key = "key%d" %slot
		if enc:match("^wep") then
			local oldkey = self.map.uci:get("wireless", section, key)
			if oldkey and #oldkey >= 10 then
				self.map.uci:set("wireless", section, key, oldkey)
			else
				self.map.uci:set("wireless", section, key, default_wep_key(slot))
			end
		end
		return nil
	end

	function wepkey.write(self, section, value)
		if value and (#value == 5 or #value == 13) then
			value = value
		end
		return Value.write(self, section, value)
	end
end

------------------- Anyfi.net interface configuration --------------------

if fs.access("/sbin/myfid") and anyfi_controller and anyfi_controller ~= "" then
	anyfi_status = s:taboption("anyfi", Flag, "anyfi_disabled", translate("Enable Anyfi.net"), translate("Enable remote access to this wireless network"))
	anyfi_status.enabled = 0
	anyfi_status.disabled = 1
	anyfi_status.default = anyfi_status.enabled
	anyfi_status.rmempty = true
	anyfi_status:depends({mode="ap", encryption="psk"})
	anyfi_status:depends({mode="ap", encryption="psk2"})
	anyfi_status:depends({mode="ap", encryption="mixed-psk"})
	anyfi_status:depends({mode="ap", encryption="wpa"})
	anyfi_status:depends({mode="ap", encryption="wpa2"})
	anyfi_status:depends({mode="ap", encryption="mixed-wpa"})

	function anyfi_status.remove(self, section)
		wdev:set("anyfi_disabled", nil)
		self.map:del(section, "anyfi_disabled")
	end

	function anyfi_status.write(self, section, value)
		wdev:set("anyfi_disabled", value)
		self.map:set(section, "anyfi_disabled", value)
		for _, net in ipairs(wdev:get_wifinets()) do
			if net:get("anyfi_disabled") ~= "1" then
				wdev:set("anyfi_disabled", nil)
				break
			end
		end
	end
end

return m
