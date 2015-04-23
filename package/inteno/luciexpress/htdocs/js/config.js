/*
 * juci - javascript universal client interface
 *
 * Project Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 * 
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
 
angular.module("luci")
.factory('$config', function($rootScope){
	return {
		mode: "basic", // basic or expert supported
		model: "Inteno DG301",
		languages: {
			"tr": {
				title: "Turkish"
			}, 
			"en": {
				title: "English"
			},
			"de": {
				title: "German"
			}, 
			"se": {
				title: "Swedish"
			}
		},
		themes: {
			"default": "/themes/default/",
			"red" : "/themes/inteno-red/",
			"vodaphone" : "/themes/vodaphone/"
		}, 
		plugins: [
			"hello_world", 
			"core", 
			"phone", 
			"router", 
			"wifi"
		], 
		rpc: {
			//host: "", not used anymore because we now instead do rpc forwarding in server.js!
			exposed_calls: [
				"session.login", 
				"session.access", 
				"session.destroy", 
				"luci2.ui.menu", 
				"luci2.network.conntrack_count",
				"luci2.network.dhcp_leases",
				"luci2.system.diskfree", 
				"router.dslstats",
				"router.info",
				"router.clients", 
				"uci.state", 
				"uci.set", 
				"uci.commit", 
				"uci.configs", 
				"network.interface.status", 
				"system.info"
			]
		}
	}; 
}); 
