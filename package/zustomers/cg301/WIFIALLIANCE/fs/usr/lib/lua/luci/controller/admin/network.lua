--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

]]--

module("luci.controller.admin.network", package.seeall)

function index()
	local uci = require("luci.model.uci").cursor()
	local users = { "admin", "support", "user" }
	local page

	for k, user in pairs(users) do
		page = node(user, "network")
		page.target = firstchild()
		page.title  = _("Network")
		page.order  = 50
		page.index  = true

	--	if page.inreq then
			local has_switch = false

			uci:foreach("network", "switch",
				function(s)
					has_switch = true
					return false
				end)

			if has_switch then
				page  = node(user, "network", "vlan")
				page.target = cbi("admin_network/vlan")
				page.title  = _("Switch")
				page.order  = 20

				page = entry({user, "network", "switch_status"}, call("switch_status"), nil)
				page.leaf = true
			end


			local has_wifi = false

			uci:foreach("wireless", "wifi-device",
				function(s)
					has_wifi = true
					return false
				end)

			if has_wifi then
				page = entry({user, "network", "wireless_join"}, call("wifi_join"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_onoff"}, call("wifi_onoff"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_add"}, call("wifi_add"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_delete"}, call("wifi_delete"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_status"}, call("wifi_status"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_reconnect"}, call("wifi_reconnect"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_shutdown"}, call("wifi_shutdown"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless_scan"}, template("admin_network/wifi_scan"), nil)
				page.leaf = true

				page = entry({user, "network", "wps"}, call("wps_setup"), nil)
				page.leaf = true

				page = entry({user, "network", "wireless"}, arcombine(template("admin_network/wifi_overview"), cbi("admin_network/wifi")), _("Wireless"), 15)
				page.leaf = true
				page.subindex = true

				--if page.inreq then
					local wdev
					entry({user, "network", "wireless", "wireless"}, alias(user, "network", "wireless"), "Wireless", 1)
					local net = require "luci.model.network".init(uci)
					for _, wdev in ipairs(net:get_wifidevs()) do
						local wnet
						for _, wnet in ipairs(wdev:get_wifinets()) do
							entry(
								{user, "network", "wireless", wnet:id()},
								alias(user, "network", "wireless"),
								wnet:ifname() .. ": " .. wnet:shortname()
							)
						end
					end
				--end
			end

			if user == "admin" then
				page = entry({user, "network", "iface_add"}, cbi("admin_network/iface_add"), nil)
				page.leaf = true

				page = entry({user, "network", "iface_delete"}, call("iface_delete"), nil)
				page.leaf = true
			end

			page = entry({user, "network", "iface_status"}, call("iface_status"), nil)
			page.leaf = true

			page = entry({user, "network", "iface_reconnect"}, call("iface_reconnect"), nil)
			page.leaf = true

			page = entry({user, "network", "iface_shutdown"}, call("iface_shutdown"), nil)
			page.leaf = true

			page = entry({user, "network", "network"}, arcombine(cbi("admin_network/network"), cbi("admin_network/ifaces")), _("Interfaces"), 10)
			page.leaf   = true
			page.subindex = true

			--if page.inreq then
				entry({user, "network", "network", "network"}, true, "Interfaces", 1)
				uci:foreach("network", "interface",
					function (section)
						local ifc = section[".name"]
						if ifc ~= "loopback" then
							if user == "admin" or section.is_lan == "1" then
								entry({user, "network", "network", ifc},
								true, ifc:upper())
							end
						end
					end)
			--end

			if nixio.fs.access("/etc/config/dhcp") then
				page = node(user, "network", "dhcp")
				page.target = cbi("admin_network/dhcp")
				page.title  = _("DHCP and DNS")
				page.order  = 30

				page = entry({user, "network", "dhcplease_status"}, call("lease_status"), nil)
				page.leaf = true

				page = node(user, "network", "hosts")
				page.target = cbi("admin_network/hosts")
				page.title  = _("Hostnames")
				page.order  = 40
			end

			if user ~= "user" then
				if nixio.fs.access("/etc/config/6relayd") then
					page = node(user, "network", "ipv6")
					page.target = cbi("admin_network/ipv6")
					page.title  = _("IPv6 RA and DHCPv6")
					page.order  = 45
				end
			end

			page  = node(user, "network", "routes")
			page.target = cbi("admin_network/routes")
			page.title  = _("Static Routes")
			page.order  = 50

			page = node(user, "network", "diagnostics")
			page.target = template("admin_network/diagnostics")
			page.title  = _("Diagnostics")
			page.order  = 60

			page = entry({user, "network", "diag_ping"}, call("diag_ping"), nil)
			page.leaf = true

			page = entry({user, "network", "diag_nslookup"}, call("diag_nslookup"), nil)
			page.leaf = true

			page = entry({user, "network", "diag_traceroute"}, call("diag_traceroute"), nil)
			page.leaf = true

			page = entry({user, "network", "diag_ping6"}, call("diag_ping6"), nil)
			page.leaf = true

			page = entry({user, "network", "diag_traceroute6"}, call("diag_traceroute6"), nil)
			page.leaf = true
	--	end

			if nixio.fs.access("/etc/config/netmode") then
				page = entry({user, "network", "setup"}, call("change_net_setup"), nil)
				page.leaf = true
			end
	end
end

function change_net_setup()
	local utl = require "luci.util"
	local uci = require "luci.model.uci".cursor()
	local mode = luci.http.formvalue("mode")
	local ssid = luci.http.formvalue("ssid")
	local key = luci.http.formvalue("key")
	local guser = luci.dispatcher.context.path[1]
	local curmode = uci:get("netmode", "setup", "curmode")
	local desc = uci:get("netmode", mode, "desc")
	local conf = uci:get("netmode", mode, "conf")
	local dir = uci:get("netmode", "setup", "dir") or "/etc/netmodes"
	local dualwifi = (luci.sys.exec("wlctl -i wl1 cap 2>/dev/null"):len() > 2)

	if curmode == mode then
		luci.http.redirect(luci.dispatcher.build_url("%s/network/network" %guser))
	end

	luci.sys.exec("cp %s/%s/* /etc/config/" %{dir, conf})
	uci:set("netmode", "setup", "curmode", mode)
	uci:commit("netmode")

	if not dualwifi then
		uci:delete("wireless", "wl1")
		uci:foreach("wireless", "wifi-iface",
			function(s)
				uci:set("wireless", s['.name'], "device", "wl0")
			end)
		uci:commit("wireless")
	end

	if ssid and ssid:len() > 2 then
		uci:foreach("wireless", "wifi-iface",
			function(s)
				uci:set("wireless", s['.name'], "ssid", ssid)
				if key:len() < 2 then
					uci:delete("wireless", s['.name'], "encryption")
					uci:delete("wireless", s['.name'], "cipher")
					uci:delete("wireless", s['.name'], "key")
				else
					uci:set("wireless", s['.name'], "encryption", "psk2")
					uci:set("wireless", s['.name'], "key", key)
				end
			end)
		uci:commit("wireless")
	end

	local lanaddr = uci:get("network", "lan", "ipaddr") or "192.168.1.1"
	luci.template.render("admin_system/applyreboot", {
		title = luci.i18n.translate("Rebooting..."),
		msg   = luci.i18n.translate("The system is rebooting in order to setup <em><b>%s</b></em> network mode.<br /> Wait a few minutes until you try to reconnect. It might be necessary to renew the address of your computer to reach the device again, depending on your settings." %desc),
		addr  = lanaddr
	})
	luci.sys.exec("sync")
	luci.sys.reboot()
end

function wifi_join()
	local function param(x)
		return luci.http.formvalue(x)
	end

	local function ptable(x)
		x = param(x)
		return x and (type(x) ~= "table" and { x } or x) or {}
	end

	local dev  = param("device")
	local ssid = param("join")

	if dev and ssid then
		local cancel  = (param("cancel") or param("cbi.cancel")) and true or false

		if cancel then
			luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless_join?device=" .. dev))
		else
			local cbi = require "luci.cbi"
			local tpl = require "luci.template"
			local map = luci.cbi.load("admin_network/wifi_add")[1]

			if map:parse() ~= cbi.FORM_DONE then
				tpl.render("header")
				map:render()
				tpl.render("footer")
			end
		end
	else
		luci.template.render("admin_network/wifi_join")
	end
end

function wifi_onoff(dev)
	luci.sys.exec("pidof ecobutton || /sbin/ecobutton %s" %dev)
	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
end

function wifi_add()
	local dev = luci.http.formvalue("device")
	local ntm = require "luci.model.network".init()

	dev = dev and ntm:get_wifidev(dev)

	if dev then
		local net = dev:add_wifinet({
			network	   = "lan",
			mode       = "ap",
			ssid       = "Guest",
			encryption = "none"
		})

		ntm:save("wireless")
		luci.http.redirect(net:adminlink())
	end
end

function wifi_delete(network)
	local ntm = require "luci.model.network".init()
	local wnet = ntm:get_wifinet(network)
	if wnet then
		local dev = wnet:get_device()
		local nets = wnet:get_networks()
		if dev then
			luci.sys.call("env -i /sbin/wifi down %q >/dev/null" % dev:name())
			ntm:del_wifinet(network)
			ntm:commit("wireless")
			local _, net
			for _, net in ipairs(nets) do
				if net:is_empty() then
					ntm:del_network(net:name())
					ntm:commit("network")
				end
			end
			luci.sys.call("env -i /sbin/wifi up %q >/dev/null" % dev:name())
		end
	end

	luci.http.redirect(luci.dispatcher.build_url("admin/network/wireless"))
end

function wps_setup()
	local method = luci.http.formvalue("method")
	local stapin = luci.http.formvalue("pin")
	local guser = luci.dispatcher.context.path[1]

	if method == "stapin" and stapin then
		local checksum = luci.sys.exec("wps_cmd checkpin %s | tr -d '\n'" %stapin)
		if stapin == checksum then
			luci.sys.exec("wps_cmd addenrollee wl0 sta_pin=%s" %stapin)
		elseif stapin == "12345671" then
			luci.http.redirect(luci.dispatcher.build_url("%s/network/wireless" %guser) .. "?wpspin=block")
		else
			luci.http.redirect(luci.dispatcher.build_url("%s/network/wireless" %guser) .. "?wpspin=invalid")
		end
	elseif method == "pbc" then
		luci.sys.exec("wps_cmd addenrollee wl0 pbc")
	end

	luci.http.redirect(luci.dispatcher.build_url("%s/network/wireless" %guser))
end

function iface_status(ifaces)
	local netm = require "luci.model.network".init()
	local rv   = { }

	local iface
	for iface in ifaces:gmatch("[%w%.%-_]+") do
		local net = netm:get_network(iface)
		local device = net and net:get_interface()
		if device then
			local data = {
				id         = iface,
				proto      = net:proto(),
				uptime     = net:uptime(),
				gwaddr     = net:gwaddr(),
				dnsaddrs   = net:dnsaddrs(),
				name       = device:shortname(),
				type       = device:type(),
				ifname     = device:name(),
				macaddr    = device:mac(),
				is_up      = device:is_up(),
				rx_bytes   = device:rx_bytes(),
				tx_bytes   = device:tx_bytes(),
				rx_packets = device:rx_packets(),
				tx_packets = device:tx_packets(),

				ipaddrs    = { },
				ip6addrs   = { },
				subdevices = { }
			}

			local _, a
			for _, a in ipairs(device:ipaddrs()) do
				data.ipaddrs[#data.ipaddrs+1] = {
					addr      = a:host():string(),
					netmask   = a:mask():string(),
					prefix    = a:prefix()
				}
			end
			for _, a in ipairs(device:ip6addrs()) do
				if not a:is6linklocal() then
					data.ip6addrs[#data.ip6addrs+1] = {
						addr      = a:host():string(),
						netmask   = a:mask():string(),
						prefix    = a:prefix()
					}
				end
			end

			for _, device in ipairs(net:get_interfaces() or {}) do
				data.subdevices[#data.subdevices+1] = {
					name       = device:shortname(),
					type       = device:type(),
					ifname     = device:name(),
					macaddr    = device:mac(),
					macaddr    = device:mac(),
					is_up      = device:is_up(),
					rx_bytes   = device:rx_bytes(),
					tx_bytes   = device:tx_bytes(),
					rx_packets = device:rx_packets(),
					tx_packets = device:tx_packets(),
				}
			end

			rv[#rv+1] = data
		else
			rv[#rv+1] = {
				id   = iface,
				name = iface,
				type = "ethernet"
			}
		end
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

function iface_reconnect(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifup %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Reconnected")
		return
	end

	luci.http.status(404, "No such interface")
end

function iface_shutdown(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
		luci.http.status(200, "Shutdown")
		return
	end

	luci.http.status(404, "No such interface")
end

function iface_delete(iface)
	local netmd = require "luci.model.network".init()
	local net = netmd:del_network(iface)
	if net then
		luci.sys.call("env -i /sbin/ifdown %q >/dev/null 2>/dev/null" % iface)
		luci.http.redirect(luci.dispatcher.build_url("admin/network/network"))
		netmd:commit("network")
		netmd:commit("dhcp")
		netmd:commit("wireless")
		return
	end

	luci.http.status(404, "No such interface")
end

function wifi_status(devs)
	local s    = require "luci.tools.status"
	local rv   = { }

	local dev
	for dev in devs:gmatch("[%w%.%-]+") do
		rv[#rv+1] = s.wifi_network(dev)
	end

	if #rv > 0 then
		luci.http.prepare_content("application/json")
		luci.http.write_json(rv)
		return
	end

	luci.http.status(404, "No such device")
end

local function wifi_reconnect_shutdown(shutdown, wnet)
	local netmd = require "luci.model.network".init()
	local net = netmd:get_wifinet(wnet)
	local dev = net:get_device()
	if dev and net then
--		luci.sys.call("env -i /sbin/wifi down >/dev/null 2>/dev/null")

		dev:set("disabled", nil)
		net:set("disabled", shutdown and 1 or nil)
		netmd:commit("wireless")

--		luci.sys.call("env -i /sbin/wifi up >/dev/null 2>/dev/null")
		luci.sys.call("env -i /sbin/wifi %s %s >/dev/null 2>/dev/null" %{shutdown and "disable" or "enable", net:ifname()})
		luci.http.status(200, shutdown and "Shutdown" or "Reconnected")

		return
	end

	luci.http.status(404, "No such radio")
end

function wifi_reconnect(wnet)
	wifi_reconnect_shutdown(false, wnet)
end

function wifi_shutdown(wnet)
	wifi_reconnect_shutdown(true, wnet)
end

function lease_status()
	local s = require "luci.tools.status"

	luci.http.prepare_content("application/json")
	luci.http.write('[')
	luci.http.write_json(s.dhcp_leases())
	luci.http.write(',')
	luci.http.write_json(s.dhcp6_leases())
	luci.http.write(']')
end

function switch_status(switches)
	local s = require "luci.tools.status"

	luci.http.prepare_content("application/json")
	luci.http.write_json(s.switch_status(switches))
end

function diag_command(cmd, addr)
	if addr and addr:match("^[a-zA-Z0-9%-%.:_]+$") then
		luci.http.prepare_content("text/plain")

		local util = io.popen(cmd % addr)
		if util then
			while true do
				local ln = util:read("*l")
				if not ln then break end
				luci.http.write(ln)
				luci.http.write("\n")
			end

			util:close()
		end

		return
	end

	luci.http.status(500, "Bad address")
end

function diag_ping(addr)
	diag_command("ping -c 5 -W 1 %q 2>&1", addr)
end

function diag_traceroute(addr)
	diag_command("traceroute -q 1 -w 1 -n %q 2>&1", addr)
end

function diag_nslookup(addr)
	diag_command("nslookup %q 2>&1", addr)
end

function diag_ping6(addr)
	diag_command("ping6 -c 5 %q 2>&1", addr)
end

function diag_traceroute6(addr)
	diag_command("traceroute6 -q 1 -w 2 -n %q 2>&1", addr)
end
