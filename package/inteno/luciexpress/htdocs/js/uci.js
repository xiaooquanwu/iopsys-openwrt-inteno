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

// uci module for interacting with uci tables
angular.module("luci")
.factory('$uci', function($rpc, $rootScope){
	// TODO: schemas must be supplied by the router. 
	var schemas = {
		"wifi-device": {
			schema: {
				"type": String,
				"country": String,
				"band": [ "a", "b" ],
				"bandwidth": Number,
				"channel": [ "auto", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ],
				"scantimer": Number,
				"wmm": Boolean,
				"wmm_noack": Boolean,
				"wmm_apsd": Boolean,
				"txpower": Number,
				"rateset": [ "default" ],
				"frag": Number,
				"rts": Number,
				"dtim_period": Number,
				"beacon_int": Number,
				"rxchainps": Boolean,
				"rxchainps_qt": Number,
				"rxchainps_pps": Number,
				"rifs": Boolean,
				"rifs_advert": Boolean,
				"maxassoc": Number,
				"doth": Boolean,
				"hwmode": [ "auto", "11ac" ],
				"radio": [ "on", "off" ]
			},
			defaults: {
				"type": "broadcom",
				"country": "EU\/13",
				"band": "b",
				"bandwidth": 20,
				"channel": "auto",
				"scantimer": 15,
				"wmm": 1,
				"wmm_noack": 0,
				"wmm_apsd": 0,
				"txpower": 100,
				"rateset": "default",
				"frag": 0,
				"rts": 0,
				"dtim_period": 1,
				"beacon_int": 100,
				"rxchainps": 0,
				"rxchainps_qt": 10,
				"rxchainps_pps": 10,
				"rifs": 0,
				"rifs_advert": 0,
				"maxassoc": 16,
				"doth": 0,
				"hwmode": "auto",
				"radio": "on"
			}
		}, 
		"wifi-iface": {
			schema: {
				"device": /^wl0|wl1$/,
				"network": [ "wan", "lan" ],
				"mode": [ "ap" ],
				"ssid": String,
				"encryption": /^none|wpa|wpa2|mixed-wpa|wep-shared|mixed-psk$/,
				"cipher": [ "auto" ],
				"key": String,
				"gtk_rekey": Boolean,
				"wps_pbc": Boolean,
				"wmf_bss_enable": Boolean,
				"bss_max": Number,
				"instance": Number,
				"up": Boolean,
				"disabled": Boolean,
				"macmode": [ 0, 1, 2 ],
				"macfilter": Boolean,
				"maclist": Array(/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/)
			},
			defaults: {
				"device": "wl0",
				"network": "lan",
				"mode": "ap",
				"ssid": "",
				"encryption": "mixed-psk",
				"cipher": "auto",
				"key": "",
				"gtk_rekey": 0,
				"wps_pbc": 1,
				"wmf_bss_enable": 1,
				"bss_max": 16,
				"instance": 1.2,
				"up": 1,
				"disabled": 0,
				"open": 0,
				"macfilter_mode": 2,
				"enabled": 1,
				"macfilter": 1,
				"maclist": ["00:00:00:00:00:00"],
				"macmode": 0,
				"ifname": "wl1"
			}
		}, 
		"host": {
			schema: {
				"hostname": String, 
				"macaddr": /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/
			}, 
			defaults: {
				"hostname": "", 
				"macaddr": "00:00:00:00:00:00"
			}
		}
	}; 

	function initReq(path){
		var parts = path.split("."); 
		var req = {}; 
		if(parts.length == 0) { deferred.reject(); return; }
		req.config = parts[0]; 
		if(parts.length > 1) req.section = parts[1]; 
		if(parts.length > 2) req.option = parts[2]; 
		return req; 
	}
	
	function stringify(obj) {
		var ret = {}; 
    for (var property in obj) {
			if (obj.hasOwnProperty(property)) {
				//console.log(property+": "+(typeof obj[property])+", array: "+(obj[property] instanceof Array)); 
				if(obj[property] instanceof Array){
					ret[property] = obj[property]; // skip arrays
				} else if (typeof obj[property] == "object"){
					ret[property] = stringify(obj[property]);
				} else {
					ret[property] = String(obj[property]); 
					if(ret[property] === "true") ret[property] = "1"; 
					if(ret[property] === "false") ret[property] = "0"; 
				}
			}
		}
		return ret; 
	}
	// validates a uci object against scheme using it's type and replaces 
	// invalid values with defaults. 
	function fixup_values(values, insert_defaults, path){
		// converts all strings that are numbers to actual numbers in object
		function fixup(obj, sc) {
			for (var property in obj) {
				if (obj.hasOwnProperty(property)) {
					if (typeof obj[property] == "object" && !(obj[property] instanceof Array)){
						fixup(obj[property], (sc.schema||{})[property]);
					} else {
						var num = Number(obj[property]); 
						if(!isNaN(num)) obj[property] = num; 
						var type = (sc.schema||{})[property]; 
						if(!(property in obj)) {
							obj[property] = sc.defaults[property]; 
						}
						if(type) {
							var errors = []; 
							var def = sc.defaults[property]; 
							if(type == Boolean){
								if(obj[property] == 0) obj[property] = false; 
								if(obj[property] == 1) obj[property] = true; 
							} else if(type instanceof Array && obj[property] instanceof Array){
								obj[property] = obj[property].map(function(x){
									var err = schema(type).errors(x); 
									if(err) {
										errors.push(err);
										return ((def instanceof Array)?def[0]:def);  
									} 
									return x; 
								}); 
							} else {
								var err = schema(type).errors(obj[property]); 
								if(err) {
									obj[property] = def; 
									errors.push(err); 
								}
							}
							if(errors.length){
								var name = (path||"")+".@"+obj[".type"]+"["+obj[".name"]+"]."+property; 
								console.error("UCI: Failed to validate field "+name+", resettings to: "+JSON.stringify(sc.defaults[property]+": "+JSON.stringify(errors))); 
							}
						}
					}
				}
			}
			return obj; 
		}
		
		var fixed = {}; 
		if(".type" in values){
			var sc = (schemas[values[".type"]]||{}); 
			fixed = fixup(values, sc);
			//validate(sc, values); 
		} else {
			fixed = {}; 
			Object.keys(values).map(function(k){
				var obj = values[k]; 
				if(!(".type" in obj)){
					console.log("Object missing type! ("+k+")"); 
				} else {
					var sc = (schemas[obj[".type"]]||{}); 
					fixed[k] = fixup(obj, sc); 
					//validate(sc, obj); 
				}
			}); 
		}
		return fixed; 
	}
	return {
		show: function(path){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			
			$rpc.uci.state(req).done(function(state){
				var fixed = null; 
				try {
					if(state && state.values) fixed = fixup_values(state.values); 
					else if(state && state.value) fixed = fixup_values(state.value); 
				} catch(err) { console.error(err); deferred.reject(err); }; 
				if(fixed) deferred.resolve(fixed); 
				else deferred.reject(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}, 
		set: function(path, values){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			req.values = stringify(fixup_values(values)); 
			$rpc.uci.set(req).done(function(state){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}, 
		// add a new rule to the uci config
		add: function(config, type, values){
			var deferred = $.Deferred(); 
			$rpc.uci.add({
				"config": config, 
				"type": type,
				"values": values
			}).done(function(state){
				values[".name"] = state.section; 
				deferred.resolve(state.section); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		},
		// commit config changes
		commit: function(path, values){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			$rpc.uci.commit(req).done(function(state){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		},
		// revert uncommitted changes
		revert: function(path, values){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			$rpc.uci.revert(req).done(function(state){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		},
		// rollback
		rollback: function(){
			var deferred = $.Deferred(); 
			$rpc.uci.rollback().done(function(state){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		},
		delete: function(path){
			var deferred = $.Deferred();
			var req = initReq(path);  
			$rpc.uci.delete(req).done(function(state){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
	}; 
}); 
