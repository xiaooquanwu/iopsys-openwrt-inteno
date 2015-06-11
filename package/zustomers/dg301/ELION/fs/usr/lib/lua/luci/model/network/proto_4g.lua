local netmod = luci.model.network

local guser = luci.dispatcher.context.path[1]

local _, p
for _, p in ipairs({"4g"}) do

	local proto = netmod:register_protocol(p)

	function proto.get_i18n(self)
		if p == "4g" then
			if guser == "admin" then
				return luci.i18n.translate("LTE/HSPA+")
			else
				return luci.i18n.translate("DHCP")
			end
			
		end
	end

	function proto.is_installed(self)
		if p == "4g" then
			return (nixio.fs.glob("/lib/netifd/proto/4g.sh")() ~= nil)
		end
	end

	function proto.is_semivirtual(self)
		return true
	end

	function proto.is_semifloating(self)
		return true
	end
end
