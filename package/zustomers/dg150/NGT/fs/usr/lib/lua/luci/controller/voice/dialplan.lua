--[[
    Copyright 2011 Iordan Iordanov <iiordanov (AT) gmail.com>

    This file is part of luci-pbx.

    luci-pbx is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    luci-pbx is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with luci-pbx.  If not, see <http://www.gnu.org/licenses/>.

    Modified to luci-app-voice-client
]]--

module("luci.controller.voice.dialplan", package.seeall)

function index()
	local users = { "admin" }

	for k, user in pairs(users) do
		entry({user, "services", "voice", "dialplan"},	cbi("voice/dialplan"),	"Dialplan",	5)
	end
end
