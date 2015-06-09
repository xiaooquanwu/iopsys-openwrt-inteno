--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2008 Jo-Philipp Wich <xm@leipzig.freifunk.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: samba.lua 6984 2011-04-13 15:14:42Z soma $
]]--
require("luci.tools.webadmin")
local sys = require "luci.sys"
local fs   = require "nixio.fs"
local util = require "nixio.util"
local uci = require("luci.model.uci").cursor()

local devices = {}
util.consume((fs.glob("/dev/sd*")), devices)
util.consume((fs.glob("/dev/sr*")), devices)
local size = {}
for i, dev in ipairs(devices) do
        local s = tonumber((fs.readfile("/sys/class/block/%s/size" % dev:sub(6))))
        size[dev] = s and math.floor(s / 2048)
end


m = Map("samba", translate("Network Shares"))

s = m:section(TypedSection, "samba", "Samba")
s.anonymous = true

s:option(Value, "name", translate("Hostname"))
s:option(Value, "description", translate("Description"))
s:option(Value, "workgroup", translate("Workgroup"))

intf = s:option(MultiValue, "interfaces", translate("Interfaces"), translate("the interface(s) samba will listen on; samba will not start if no interface is selected"))
intf.rmempty = true
uci:foreach("network", "interface",
	function (section)
		local ifc = section[".name"]
		local islan = section["is_lan"]
		local typ = section["type"]
		if ifc ~= "loopback" and typ ~= "alias" and islan == "1" then
			intf:value(ifc)
		end
	end)
intf.widget = "checkbox"

s = m:section(TypedSection, "sambausers", translate("Samba Users"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"
us=s:option(Value, "user", translate("User Name"))
us.rmempty  = false
us.default="samba"
pass=s:option(Value, "password", translate("Password"))
pass.password = true
pass.rmempty  = false
s:option(Value, "desc", translate("Description"))

s = m:section(TypedSection, "sambashare", translate("Shared Directories"))
s.anonymous = true
s.addremove = true
s.template = "cbi/tblsection"

s:option(Value, "name", translate("Name"))
pth = s:option(ListValue, "path", translate("Device"))

function mountpoint(dev)
	for mount in io.lines("/proc/self/mounts") do
		if mount:match("^%s" %dev) and mount:match("/mnt/") then
			return mount:match("/mnt/%S+")
		end
	end
end

for i, dev in ipairs(devices) do
        local s = tonumber((fs.readfile("/sys/class/block/%s/size" % dev:sub(6))))
	local sdahost = sys.exec("ls -l /sys/class/block/%s"  % dev:sub(6))
	local scsiid = sdahost:match("host%d")
	if scsiid then
		local usbinfo=fs.readfile("/proc/scsi/usb-storage/%s" % scsiid:match("%d"))
		size[dev] = s and math.floor(s / 2048)
		if dev:match("^/dev/sd%w%d$") or dev:match("^/dev/sr%d$") then
			if usbinfo and usbinfo:match("Vendor: [%w%s%d.,]*\n") and usbinfo:match("Product: [%w%s%d.,]*\n") then
				pth:value(mountpoint(dev), translate(usbinfo:match("Vendor: [%w%s%d.,]*\n").." "..usbinfo:match("Product: [%w%s%d.,]*\n").." size :"..size[dev].."Mb"))
			else
				pth:value(mountpoint(dev), translate(dev.." size :"..size[dev].."Mb"))
			end
		end
	end
end
	

compath = s:option(Value, "dirpath", translate("Directory"))

function dirpath_is_valid(value)
	if value:match("%.%.") then
		return false
	else
		return true
	end
end

function compath.validate(self, value, section)
    if dirpath_is_valid(value) then
        return value
    else
        return nil, "Invalid Directory"
    end
end

if nixio.fs.access("/etc/config/fstab") then
        pth.titleref = luci.dispatcher.build_url("admin", "system", "fstab")
end

s:option(Value, "users", translate("Allowed users")).rmempty = true

go = s:option(Flag, "guest_ok", translate("Allow guests"))
go.rmempty = false
go.enabled = "yes"
go.disabled = "no"

ro = s:option(Flag, "read_only", translate("Read-only"))
ro.rmempty = false
ro.enabled = "yes"
ro.disabled = "no"


cm = s:option(Value, "create_mask", translate("Create mask"),
        translate("Mask for new files"))
cm.rmempty = true
cm.default="0700"
cm.size = 4

dm = s:option(Value, "dir_mask", translate("Directory mask"),
        translate("Mask for new directories"))
dm.rmempty = true
dm.default="0700"
dm.size = 4

return m
