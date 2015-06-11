--[[
LuCI - Lua Configuration Interface

Copyright 2011-2012 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0
]]--

local nfs = require "nixio.fs"

local map, section, net = ...
local ifc = net:get_interface()

local apn, pincode, username, password
local hostname, accept_ra, send_rs
local bcast, defaultroute, peerdns, dns, metric, clientid, vendorclass

local guser = luci.dispatcher.context.path[1]


dongle = section:taboption("general", ListValue, "modem", translate("LTE Modem"))
dongle.rmempty = false
dongle:value("", translate("-- Please Choose --"))

local usbnets = "/var/usbnets"
if nfs.access(usbnets, "r") then
	local fd = io.open(usbnets, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local uNo, uBr, uVid, uPid, uMa, uPr, netdev, comdev, mdmtyp, vendor, product = ln:match("(%S+):(%S+) (%S+):(%S+) (%S+) (%S+) (%S+) (%S+) (%S+) (%S+) (%S+)")
				if vendor and product then
					dongle:value(mdmtyp..":"..comdev..":"..netdev, vendor.." "..product)
				else
					uNo, uBr, uVid, uPid, uMa, uPr, netdev, comdev, mdmtyp = ln:match("(%S+):(%S+) (%S+):(%S+) (%S+) (%S+) (%S+) (%S+) (%S+)")
					if uMa and uPr then
						dongle:value(mdmtyp..":"..comdev..":"..netdev, uMa.." "..uPr)
					end
				end
				--dongle:value(uVid..":"..uPid..":"..mdmtyp, vendor.." "..product)
			end
		end
		fd:close()
	end
end

function dongle.write(self, section, value)
	local mdmtyp, comdev, netdev = value:match("(%S+):(%S+):(%S+)")
	if netdev then
		self.map:set(section, "modem", value)
		self.map:set(section, "service", mdmtyp)
		self.map:set(section, "comdev", "/dev/"..comdev)
		self.map:set(section, "ifname", netdev)
	end
end


apn = section:taboption("general", Value, "apn", translate("APN"))


pincode = section:taboption("general", Value, "pincode", translate("PIN"))


username = section:taboption("general", Value, "username", translate("PAP/CHAP username"))


password = section:taboption("general", Value, "password", translate("PAP/CHAP password"))
password.password = true


hostname = section:taboption("general", Value, "hostname",
	translate("Hostname to send when requesting DHCP"))

hostname.placeholder = luci.sys.hostname()
hostname.datatype    = "hostname"


if guser == "admin" then
	bcast = section:taboption("advanced", Flag, "broadcast",
		translate("Use broadcast flag"),
		translate("Required for certain ISPs, e.g. Charter with DOCSIS 3"))

	bcast.default = bcast.disabled


	defaultroute = section:taboption("advanced", Flag, "defaultroute",
		translate("Use default gateway"),
		translate("If unchecked, no default route is configured"))

	defaultroute.default = defaultroute.enabled


	peerdns = section:taboption("advanced", Flag, "peerdns",
		translate("Use DNS servers advertised by peer"),
		translate("If unchecked, the advertised DNS server addresses are ignored"))

	peerdns.default = peerdns.enabled


	dns = section:taboption("advanced", DynamicList, "dns",
		translate("Use custom DNS servers"))

	dns:depends("peerdns", "")
	dns.datatype = "ipaddr"
	dns.cast     = "string"


	metric = section:taboption("advanced", Value, "metric",
		translate("Use gateway metric"))

	metric.placeholder = "0"
	metric.datatype    = "uinteger"


	clientid = section:taboption("advanced", Value, "clientid",
		translate("Client ID to send when requesting DHCP"))


	vendorclass = section:taboption("advanced", Value, "vendorid",
		translate("Vendor Class to send when requesting DHCP"))


	luci.tools.proto.opt_macaddr(section, ifc, translate("Override MAC address"))


	mtu = section:taboption("advanced", Value, "mtu", translate("Override MTU"))
	mtu.placeholder = "1500"
	mtu.datatype    = "max(9200)"
end -- guser == "admin" --
