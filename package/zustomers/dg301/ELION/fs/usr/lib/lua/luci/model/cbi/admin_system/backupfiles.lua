local uci = require("luci.model.uci").cursor()
local guser = luci.dispatcher.context.path[1]

m = Map("backup", translate("Backup Settings"))
m:append(Template("admin_system/backupfiles"))

s = m:section(TypedSection, "keep", translate("Select the services/settings to backup"))
s.anonymous = true
s.addremove = false

local name, desc
uci:foreach("backup", "service",
	function (section)
		name = section[".name"]
		desc = section["desc"]
		detail = section["detail"]
		user = section["user"]
		if (user and user == "1") or guser ~= "user" then
			k = s:option(Flag, name, desc or name, detail or "")
			k.rmempty = false
		end
	end)

return m
