--[[
LuCI - Lua Configuration Interface

Copyright 2008 Steven Barth <steven@midlink.org>
Copyright 2011 Jo-Philipp Wich <xm@subsignal.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

$Id: status.lua 9678 2013-03-07 12:17:43Z jow $
]]--

module("luci.controller.admin.status", package.seeall)

function index()
	local users = { "admin", "support", "user" }

	for k, user in pairs(users) do

		entry({user, "status"}, alias(user, "status", "overview"), _("Status"), 20).index = true
		entry({user, "status", "overview"}, template("admin_status/index"), _("Overview"), 1)
		entry({user, "status", "iptables"}, call("action_iptables"), _("Firewall"), 2).leaf = true
		entry({user, "status", "routes"}, template("admin_status/routes"), _("Routes"), 3)

		entry({user, "status", "logs"}, call("action_syslog"), _("Logs and Info"), 4).subindex = true
		entry({user, "status", "logs", "syslog"}, call("action_syslog"), _("System Log"), 4)
		entry({user, "status", "logs", "dmesg"}, call("action_dmesg"), _("Kernel Log"), 5)
		entry({user, "status", "logs", "tr069log"}, call("action_tr069log"), _("TR-069 Log"), 10)
		entry({user, "status", "logs", "packageinfo"}, call("action_packageinfo"), _("Package Info"), 20)

		entry({user, "status", "processes"}, cbi("admin_status/processes"), _("Processes"), 20)

		entry({user, "status", "realtime"}, alias(user, "status", "realtime", "load"), _("Realtime Graphs"), 30)

		entry({user, "status", "realtime", "load"}, template("admin_status/load"), _("Load"), 1).leaf = true
		entry({user, "status", "realtime", "load_status"}, call("action_load")).leaf = true

		entry({user, "status", "realtime", "bandwidth"}, template("admin_status/bandwidth"), _("Traffic"), 2).leaf = true
		entry({user, "status", "realtime", "bandwidth_status"}, call("action_bandwidth")).leaf = true

		entry({user, "status", "nameinfo"}, call("action_nameinfo")).leaf = true
	end
end

function action_syslog()
	local syslog = luci.sys.syslog()
	luci.template.render("admin_status/syslog", {syslog=syslog})
end

function action_dmesg()
	local dmesg = luci.sys.dmesg()
	luci.template.render("admin_status/dmesg", {dmesg=dmesg})
end

function action_tr069log()
	local tr069log = luci.sys.exec("cat /var/log/cwmpd.log 2>/dev/null")
	if tr069log:len() < 5 then
		tr069log = "TR-069 log not available"
	end

	luci.template.render("admin_status/tr069log", {tr069log=tr069log})
end

function action_packageinfo()
	local packageinfo = luci.sys.exec("opkg list 2>/dev/null")
	luci.template.render("admin_status/packageinfo", {packageinfo=packageinfo})
end

function action_iptables()
	local guser = luci.dispatcher.context.path[1] 
	if luci.http.formvalue("zero") then
		if luci.http.formvalue("zero") == "6" then
			luci.util.exec("ip6tables -Z")
		else
			luci.util.exec("iptables -Z")
		end
		luci.http.redirect(
			luci.dispatcher.build_url(guser, "status", "iptables")
		)
	elseif luci.http.formvalue("restart") == "1" then
		luci.util.exec("/etc/init.d/firewall reload")
		luci.http.redirect(
			luci.dispatcher.build_url(guser, "status", "iptables")
		)
	else
		luci.template.render("admin_status/iptables")
	end
end

function action_bandwidth(iface)
	luci.http.prepare_content("application/json")

	local bwc = io.popen("luci-bwc -i %q 2>/dev/null" % iface)
	if bwc then
		luci.http.write("[")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end
end

function action_wireless(iface)
	luci.http.prepare_content("application/json")

	local bwc = io.popen("luci-bwc -r %q 2>/dev/null" % iface)
	if bwc then
		luci.http.write("[")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end
end

function action_load()
	luci.http.prepare_content("application/json")

	local bwc = io.popen("luci-bwc -l 2>/dev/null")
	if bwc then
		luci.http.write("[")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end
end

function action_connections()
	local sys = require "luci.sys"

	luci.http.prepare_content("application/json")

	luci.http.write("{ connections: ")
	luci.http.write_json(sys.net.conntrack())

	local bwc = io.popen("luci-bwc -c 2>/dev/null")
	if bwc then
		luci.http.write(", statistics: [")

		while true do
			local ln = bwc:read("*l")
			if not ln then break end
			luci.http.write(ln)
		end

		luci.http.write("]")
		bwc:close()
	end

	luci.http.write(" }")
end

function action_nameinfo(...)
	local i
	local rv = { }
	for i = 1, select('#', ...) do
		local addr = select(i, ...)
		local fqdn = nixio.getnameinfo(addr)
		rv[addr] = fqdn or (addr:match(":") and "[%s]" % addr or addr)
	end

	luci.http.prepare_content("application/json")
	luci.http.write_json(rv)
end
