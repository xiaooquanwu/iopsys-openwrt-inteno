local sys = require "luci.sys"

local PORTS = { "eth0", "eth1", "eth2", "eth3", "eth4" }

local guser = luci.dispatcher.context.path[1]

m = Map("ports", translate("Port Management"))

s = m:section(TypedSection, "ethports", "Ethernet Ports",
"Auto-negation for Gigabit ports (WAN and GbE) is 10-1000 Mb Auto Mode")

s.anonymous = true

function interfacename(port)
	local intfname = sys.exec(". /lib/network/config.sh && interfacename %s" %port)
	if intfname:len() > 0 then
		return intfname
	end
	return nil
end

function is_ethwan(port)
	return (sys.exec("uci get layer2_interface_ethernet.Wan.baseifname"):match(port))
end

for _, eport in ipairs(PORTS) do
	if interfacename(eport) then
		if guser == "admin" or not sys.exec("uci -q get ports.@ethports[0].%s" %eport):match("disabled") then
			eth = s:option(ListValue, eport, "%s (%s)" %{eport, interfacename(eport)})
			eth.rmempty = true
			eth:value("auto", "Auto-negotiation")
			--eth:value("1000FD", "1000Mb, Full Duplex")
			--eth:value("1000HD", "1000Mb, Half Duplex")
			eth:value("100FD", "100Mb, Full Duplex")
			eth:value("100HD", "100Mb, Half Duplex")
			eth:value("10FD" , "10Mb,  Full Duplex")
			eth:value("10HD" , "10Mb,  Half Duplex")
			if guser == "admin" then
				eth:value("disabled" , "Disabled")
			end
		end
	end
end

return m

