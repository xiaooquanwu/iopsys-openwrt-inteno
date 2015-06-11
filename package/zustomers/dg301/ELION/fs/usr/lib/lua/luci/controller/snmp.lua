
module("luci.controller.snmp", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/snmpd") then
		return
	end

	local users = { "admin", "support", "user" }
        local page

	for k, user in pairs(users) do

		page = node(user, "system", "snmp")
		page.target = cbi("snmp/snmp")
		page.title  = _("SNMP")
		page.subindex = true
		page.dependent = true
		page.order = 82

		entry({user, "system", "snmp", "snmp"}, cbi("snmp/snmp"), "SNMP", 1)
		entry({user, "system", "snmp", "system"}, cbi("snmp/system"), "System", 2)
		entry({user, "system", "snmp", "agent"}, cbi("snmp/agent"), "Agent", 3)
		entry({user, "system", "snmp", "com2sec"}, cbi("snmp/com2sec"), "Com2sec", 4)
		entry({user, "system", "snmp", "group"}, cbi("snmp/group"), "Group", 5)
		entry({user, "system", "snmp", "view"}, cbi("snmp/view"), "View", 6)
		entry({user, "system", "snmp", "access"}, cbi("snmp/access"), "Access", 7)
		entry({user, "system", "snmp", "pass"}, cbi("snmp/pass"), "Pass", 8)
		entry({user, "system", "snmp", "exec"}, cbi("snmp/exec"), "Exec", 9)
	end
end
