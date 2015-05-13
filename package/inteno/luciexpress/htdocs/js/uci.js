//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

(function(scope){
	//var JUCI = exports.JUCI; 
	var $rpc = scope.UBUS; 
	
	// just for the extractor
	var gettext = function(str) { return str; }; 
	
	function DefaultValidator(){
		this.validate = function(field){
			return null; // return null to signal that there was no error
		}
	}
	
	function TimespanValidator(){
		this.validate = function(field){
			var parts = field.value.split("-"); 
			if(parts.length != 2) return gettext("Please specify both start time and end time for schedule!"); 
			function split(value) { return value.split(":").map(function(x){ return Number(x); }); };
			var from = split(parts[0]);
			var to = split(parts[1]); 
			if(from[0] >= 0 && from[0] < 24 && to[0] >= 0 && to[0] < 24 && from[1] >= 0 && from[1] < 60 && to[1] >= 0 && to[1] < 60){
				if((from[0]*60+from[1]) < (to[0]*60+to[1])) {
					return null; 
				} else {
					return gettext("Schedule start time must be lower than schedule end time!"); 
				}
			} else {
				return gettext("Please enter valid time value for start and end time!"); 
			}
		}
	}
	
	function WeekDayListValidator(){
		this.validate = function(field){
			if(!field.schema.allow) return null; 
			var days_valid = field.value.filter(function(x){
				return field.schema.allow.indexOf(x) != -1; 
			}).length; 
			if(!days_valid) return gettext("Please pick days between mon-sun"); 
			return null; 
		}
	}

	function PortValidator(){
		var PORT_REGEX = /^\d{1,5}$/;

		this.validate = function(field){
			if(field.value == undefined || !field.value.split) return null; 
			var parts = field.value.split("-");
			if (parts.length > 1) {  // check if it is a port range or not
				for (var i = 0; i < parts.length; i++) {
					var outcome = this.validatePort(parts[i]);
					if (outcome != null) { return outcome; }
				}
			} else {
				return this.validatePort(parts)
			}
		};

		this.validatePort = function(port) {
			if (PORT_REGEX.test(port)) { // valid regex
				if (port <= 0 || port > 65535) {
					return gettext("Port value must be between 0 and 65536");
				} else {
					return null;
				}
			} else {
				return gettext("Port value is invalid");
			}
		}
	}
	
	var section_types = {
		"juci": {
			"settings": {
				"theme":					{ dvalue: "", type: String }, 
				"lang":						{ dvalue: "", type: String }, 
				"themes":					{ dvalue: [], type: Array }, 
				"plugins":				{ dvalue: [], type: Array }, 
				"languages":			{ dvalue: [], type: Array }
			}
		}, 
		"easybox": {
			"easybox-settings": {
				"usb_port": 		{ dvalue: true, type: Boolean }, 
				"status_led": 	{ dvalue: true, type: Boolean }, 
				"power_led": 		{ dvalue: true, type: Boolean }, 
				"power_led_br":	{ dvalue: 100, type: Number },
				"wifibutton": 	{ dvalue: true, type: Boolean },
				"wpsbutton": 		{ dvalue: true, type: Boolean },
				"wpsdevicepin": { dvalue: true, type: Boolean }
			},
			"easybox-services": {
				"internet":				{ dvalue: "", type: String },
				"voice":					{ dvalue: "", type: String },
				"iptv":						{ dvalue: "", type: String }
			}
		}, 
		"firewall": {
			"defaults": {
				"syn_flood":		{ dvalue: true, type: Boolean }, 
				"intput":				{ dvalue: "ACCEPT", type: String }, 
				"output":				{ dvalue: "ACCEPT", type: String }, 
				"forward":			{ dvalue: "REJECT", type: String }, 
			}, 
			"zone": {
				"name":					{ dvalue: "", type: String }, 
				"input":				{ dvalue: "ACCEPT", type: String }, 
				"output":				{ dvalue: "ACCEPT", type: String }, 
				"forward":			{ dvalue: "REJECT", type: String }, 
				"network": 			{ dvalue: [], type: Array }, 
				"masq":					{ dvalue: true, type: Boolean }, 
				"mtu_fix": 			{ dvalue: true, type: Boolean }
			}, 
			"redirect": {
				"src_ip":				{ dvalue: "", type: String },
				"src_dport":		{ dvalue: 0, type: String, validator: PortValidator },
				"proto":				{ dvalue: "tcp", type: String }, 
				"dest_ip":			{ dvalue: "", type: String }, 
				"dest_port":		{ dvalue: 0, type: String, validator: PortValidator }
			}, 
			"rule": {
				"name":					{ dvalue: "", type: String }, 
				"src":					{ dvalue: "lan", type: String }, 
				"src_ip":				{ dvalue: "", type: String }, 
				"src_port":			{ dvalue: 0, type: Number }, 
				"proto":				{ dvalue: "tcp", type: String }, 
				"dest":					{ dvalue: "*", type: String }, 
				"dest_ip":			{ dvalue: "", type: String }, 
				"dest_port":		{ dvalue: 0, type: Number }, 
				"target":				{ dvalue: "REJECT", type: String }, 
				"family": 			{ dvalue: "ipv4", type: String }, 
				"icmp_type": 		{ dvalue: [], type: Array },
				"enabled": 			{ dvalue: true, type: Boolean },
				"hidden": 			{ dvalue: true, type: Boolean }
			}, 
			"settings": {
				"disabled":			{ dvalue: false, type: Boolean },
				"ping_wan":			{ dvalue: false, type: Boolean }
			}
		}, 
		"system": {
			"system": {
				"timezone":				{ dvalue: '', type: String },
				"zonename":				{ dvalue: '', type: String },
				"conloglevel":		{ dvalue: 0, type: Number },
				"cronloglevel":		{ dvalue: 0, type: Number },
				"hostname":				{ dvalue: '', type: String },
				"displayname":		{ dvalue: '', type: String },
				"log_size":				{ dvalue: 200, type: Number }
			}
		}, 
		"voice_client": {
			"brcm_line": {
				"extension": 			{ dvalue: '', type: String }, 
				"sip_account": 		{ dvalue: '', type: String }, 
				"noise":					{ dvalue: false, type: Boolean }, 
				"vad":						{ dvalue: false, type: Boolean }, 
				"txgain":					{ dvalue: false, type: Boolean }, 
				"rxgain":					{ dvalue: false, type: Boolean }, 
				"echo_cancel":		{ dvalue: true, type: Boolean }, 
				"callwaiting":		{ dvalue: false, type: Boolean }, 
				"clir":						{ dvalue: false, type: Boolean }, 
				"name":						{ dvalue: '', type: String }, 
				"instance":				{ dvalue: '', type: String }
			}, 
			"sip_service_provider": {           
				"name":							{ dvalue: "Account 1", type: String },
				"codec0":						{ dvalue: "alaw", type: String },  
				"codec1":						{ dvalue: "ulaw", type: String },       
				"codec2":						{ dvalue: "g729", type: String },   
				"codec3":						{ dvalue: "g726", type: String },
				"autoframing":			{ dvalue: false, type: Boolean },
				"cfim_on":					{ dvalue: "*21*", type: String },  
				"cfim_off":					{ dvalue: "#21#", type: String },
				"cfbs_on":					{ dvalue: "*61*", type: String }, 
				"cfbs_off":					{ dvalue: "#61#", type: String }, 
				"call_return":			{ dvalue: "*69", type: String },         
				"redial":						{ dvalue: "*66", type: String },   
				"is_fax":						{ dvalue: false, type: Boolean },      
				"transport":				{ dvalue: "udp", type: String },
				"priority_ulaw":		{ dvalue: 0, type: Number },
				"priority_alaw":		{ dvalue: 0, type: Number }, 
				"priority_g729":		{ dvalue: 0, type: Number },  
				"priority_g723":		{ dvalue: 0, type: Number },
				"priority_g726":		{ dvalue: 0, type: Number },   
				"enabled":					{ dvalue: true, type: Boolean },
				"target":						{ dvalue: "direct", type: String },       
				"call_lines":				{ dvalue: "BRCM/4", type: String },
				"mailbox":					{ dvalue: "-", type: String },     
				"call_filter":			{ dvalue: "-", type: String },     
				"domain":						{ dvalue: "217.27.161.62", type: String },      
				"user":							{ dvalue: "0854601910", type: String }, 
				"authuser":					{ dvalue: "0854601910", type: String }, 
				"displayname":			{ dvalue: "TEST", type: String }, 
				"ptime_ulaw":				{ dvalue: 20, type: Number }, 
				"ptime_g726":				{ dvalue: 20, type: Number },     
				"ptime_g729":				{ dvalue: 20, type: Number }, 
				"ptime_alaw":				{ dvalue: 20, type: Number }, 
				"host":							{ dvalue: "217.27.161.62", type: String },  
				"outboundproxy":		{ dvalue: "217.27.161.62", type: String }
			}
		}, 
		"wireless": {
			"wifi-status": {
				"wlan":		{ dvalue: true, type: Boolean }, 
				"wps":		{ dvalue: true, type: Boolean },
				"schedule":	{ dvalue: false, type: Boolean },
				"sched_status":	{ dvalue: false, type: Boolean }
			},
			"wifi-schedule": {
				"days":		{ dvalue: [], type: Array, allow: ["mon", "tue", "wed", "thu", "fri", "sat", "sun"], validator: WeekDayListValidator},
				"time":		{ dvalue: "", type: String, validator: TimespanValidator}
			},
			"wifi-device": {
				"type": 			{ dvalue: "", type: String },
				"country": 		{ dvalue: "", type: String},
				"band": 			{ dvalue: "none", type: String, allow: [ "a", "b" ] },
				"bandwidth": 	{ dvalue: 0, type: String, allow: [ "20", "40", "80" ] },
				"channel":		{ dvalue: "auto", type: String, allow: [ "auto", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48 ] },
				"scantimer":	{ dvalue: 0, type: Number },
				"wmm":				{ dvalue: false, type: Boolean },
				"wmm_noack":	{ dvalue: false, type: Boolean },
				"wmm_apsd":		{ dvalue: false, type: Boolean },
				"txpower":		{ dvalue: 0, type: Number },
				"rateset":		{ dvalue: "default", type: String, allow: [ "default" ] },
				"frag":				{ dvalue: 0, type: Number },
				"rts":				{ dvalue: 0, type: Number },
				"dtim_period":{ dvalue: 0, type: Number },
				"beacon_int":	{ dvalue: 0, type: Number },
				"rxchainps":	{ dvalue: false, type: Boolean },
				"rxchainps_qt":{ dvalue: 0, type: Number },
				"rxchainps_pps":{ dvalue: 0, type: Number },
				"rifs":				{ dvalue: false, type: Boolean },
				"rifs_advert":{ dvalue: false, type: Boolean },
				"maxassoc":		{ dvalue: 0, type: Number },
				"doth":				{ dvalue: 0, type: Boolean },
				"hwmode":			{ dvalue: "auto", type: String, allow: [ "auto", "11a", "11n", "11ac" ] },
				"disabled":		{ dvalue: false, type: Boolean }
			}, 
			"wifi-iface": {
				"device": 		{ dvalue: "wl0", type: String, match: /^wl0|wl1$/ },
				"network":		{ dvalue: "lan", type: String, allow: [ "lan", "guest" ] },
				"mode":				{ dvalue: "ap", type: String, allow: [ "ap" ] },
				"ssid":				{ dvalue: "Inteno", type: String },
				"encryption":	{ dvalue: "mixed-psk", type: String, allow: [ "none", "wep", "psk", "psk2", "mixed-psk" ] },
				"cipher":			{ dvalue: "auto", type: String, allow: [ "auto" ] },
				"key":				{ dvalue: "", type: String },
				"gtk_rekey":	{ dvalue: false, type: Boolean },
				"wps_pbc":		{ dvalue: false, type: Boolean },
				"wmf_bss_enable":{ dvalue: false, type: Boolean },
				"bss_max":		{ dvalue: 0, type: Number },
				"instance":		{ dvalue: 0, type: Number },
				"up":					{ dvalue: false, type: Boolean },
				"closed":			{ dvalue: false, type: Boolean },
				"disabled":		{ dvalue: false, type: Boolean },
				"macmode":		{ dvalue: 1, type: Number, allow: [ 0, 1, 2 ] },
				"macfilter":	{ dvalue: false, type: Boolean },
				"maclist":		{ dvalue: [], type: Array, match_each: /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/ }
			}
		},
		"network": {
			"interface": {
                "is_lan":       {dvalue: 1, type: Number},
                "ifname":       {dvalue: "", type: String},
                "proto":        {dvalue: "dhcp", type: String},
                "ipaddr":       {dvalue: "", type: String},
                "netmask":      {dvalue: "", type: String},
                "peerdns":      {dvalue: false, type: String},
                "dns":          {dvalue: [], type: Array}
            }
		},
        //"ddns": {
        //    "interface":            { dvalue: "", type: String },
        //    "enabled":              { dvalue: 0, type: Number },
        //    "service_name":         { dvalue: "", type: String },
        //    "domain":               { dvalue: "", type: String },
        //    "username":             { dvalue: "", type: String },
        //    "password":             { dvalue: "", type: String }
        //},
		"unknown": {
			"host": {
				"hostname":		{ dvalue: "", type: String, required: true}, 
				"macaddr":		{ dvalue: "", type: String, match: /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/, required: true}
			},
			"upgrade": {
				"fw_check_url":		{ dvalue: "", type: String, required: false},
				"fw_path_url":		{ dvalue: "", type: String, required: false},
				"fw_find_ext":		{ dvalue: "", type: String, required: false},
				"fw_down_path":		{ dvalue: "", type: String, required: false}
			}
		}
	}; 
	function UCI(){
		
	}
	(function(){
		function UCIField(value, schema){
			if(!schema) throw new Error("No schema specified for the field!"); 
			this.ovalue = value; 
			if(value != null && value instanceof Array) {
				this.ovalue = []; Object.assign(this.ovalue, value); 
			} 
			this.dirty = false; 
			this.uvalue = undefined; 
			this.schema = schema; 
			if(schema.validator) this.validator = new schema.validator(); 
			else this.validator = new DefaultValidator(); 
		}
		UCIField.prototype = {
			$reset: function(value){
				this.ovalue = this.uvalue = value; 
				if(value != null && value instanceof Array) {
					this.ovalue = []; Object.assign(this.ovalue, value); 
				}
				this.dirty = false; 
			}, 
			get value(){
				if(this.uvalue == undefined) return this.ovalue;
				else return this.uvalue; 
			},
			set value(val){
				if(!this.dirty && this.ovalue != val) this.dirty = true; 
				this.uvalue = val; 
			},
			get error(){
				return this.validator.validate(this); 
			},
			get valid(){
				return this.validator.validate(this) == null; 
			}
		}
		UCI.Field = UCIField; 
	})(); 
	(function(){
		function UCISection(config){
			this[".config"] = config; 
		}
		
		UCISection.prototype.$update = function(data){
			if(!(".type" in data)) throw new Error("Supplied object does not have required '.type' field!"); 
			// try either <config>-<type> or just <type>
			var type = 	section_types[this[".config"][".name"]][data[".type"]]; 
			if(!type) {
				console.error("Section.$update: unrecognized section type "+this[".config"][".name"]+"-"+data[".type"]); 
				return; 
			}
			var self = this; 
			self[".original"] = data; 
			self[".name"] = data[".name"]; 
			self[".type"] = data[".type"]; 
			self[".section_type"] = type; 
			
			Object.keys(type).map(function(k){
				var field = self[k]; 
				if(!field) { field = self[k] = new UCI.Field("", type[k]); }
				var value = type[k].dvalue; 
				if(!(k in data)) { 
					//console.log("Field "+k+" missing in data!"); 
				} else {
					switch(type[k].type){
						case String: value = data[k]; break; 
						case Number: 
							var n = Number(data[k]); 
							if(isNaN(n)) n = type.dvalue;
							value = n; 
							break; 
						case Array: value = data[k];  break; 
						case Boolean: 
							if(data[k] === "true" || data[k] === "1") value = true; 
							else if(data[k] === "false" || data[k] === "0") value = false; 
							break; 
						default: 
							value = data[k]; 
					}
				}
				field.$reset(value); 
			}); 
		}
		
		UCISection.prototype.$sync = function(){
			var deferred = $.Deferred(); 
			var self = this; 

			$rpc.uci.state({
				config: self[".config"][".name"], 
				section: self[".name"]
			}).done(function(data){
				self.$update(data.values);
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
		
		UCISection.prototype.$save = function(){
			var deferred = $.Deferred(); 
			var self = this; 
			
			$rpc.uci.set({
				config: self[".config"][".name"], 
				section: self[".name"], 
				values: self.$getChangedValues()
			}).done(function(data){
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
		
		UCISection.prototype.$delete = function(){
			var self = this; 
			if(self[".config"]) return self[".config"].$deleteSection(self); 
			var def = $.Deferred(); def.reject(); 
			return def.promise(); 
		}
		
		UCISection.prototype.$getErrors = function(){
			var errors = []; 
			var self = this; 
			var type = self[".section_type"]; 
			Object.keys(type).map(function(k){
				if(self[k] && self[k].error){
					errors.push(self[k].error); 
				}
			}); 
			return errors; 
		}
		
		UCISection.prototype.$getChangedValues = function(){
			var type = this[".section_type"]; 
			if(!type) return {}; 
			var self = this; 
			var changed = {}; 
			Object.keys(type).map(function(k){
				if(self[k] && self[k].dirty){ 
					//console.log("Adding dirty field: "+k); 
					changed[k] = self[k].value; 
				}
			}); 
			return changed; 
		}
		UCI.Section = UCISection; 
	})(); 
	(function(){
		function UCIConfig(uci, name){
			var self = this; 
			self.uci = uci; 
			self[".name"] = name; 
			self["@all"] = []; 
			// set up slots for all known types of objects so we can reference them in widgets
			Object.keys(section_types[name]||{}).map(function(type){
				self["@"+type] = []; 
			}); 
			//this["@deleted"] = []; 
		}
		
		function _insertSection(self, item){
			console.log("Adding local section: "+self[".name"]+"."+item[".name"]); 
			var section = new UCI.Section(self); 
			section.$update(item); 
			var type = "@"+item[".type"]; 
			if(!(type in self)) self[type] = []; 
			self[type].push(section); 
			self["@all"].push(section); 
			self[item[".name"]] = section; 
			return section; 
		}
		function _updateSection(self, item){
			var section = self[item[".name"]]; 
			if(section && section.$update) section.$update(item); 
		}
		
		function _unlinkSection(self, section){
			// NOTE: can not use filter() because we must edit the list in place 
			// in order to play well with controls that reference the list! 
			console.log("Removing local section: "+self[".name"]+"."+section[".name"]+" - "+section[".type"]); 
			var all = self["@all"]; 
			for(var i = 0; i < all.length; i++){
				if(all[i][".name"] === section[".name"]) {
					all.splice(i, 1); 
					break; 
				}; 
			}
			var jlist = self["@"+section[".type"]]||[]; 
			for(var j = 0; j < jlist.length; j++){
				if(jlist[j][".name"] === section[".name"]) {
					jlist.splice(j, 1); 
					break; 
				}
			}
			if(section[".name"]) delete self[section[".name"]]; 
		}
		
		UCIConfig.prototype.$sync = function(){
			var deferred = $.Deferred(); 
			var self = this; 

			$rpc.uci.state({
				config: self[".name"]
			}).done(function(data){
				var vals = data.values;
				Object.keys(vals).map(function(k){
					if(!(k in self)) _insertSection(self, vals[k]); 
					else _updateSection(self, vals[k]); 
				}); 
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
		// set object values on objects that match search criteria 
		// if object does not exist, then create a new object 
		UCIConfig.prototype.set = function(search, values){
			var self = this; 
			self["@all"].map(function(item){
				var match = Object.keys(search).filter(function(x){ item[x] != search[x]; }).length == 0; 
				if(match){
					Object.keys(values).map(function(x){
						item[x].value = values[x]; 
					}); 
				}
			}); 
		}
		
		UCIConfig.prototype.$deleteSection = function(section){
			var self = this; 
			var deferred = $.Deferred(); 
			console.log("Deleting section "+self[".name"]+"."+section[".name"]); 
			
			//self[".need_commit"] = true; 
			$rpc.uci.delete({
				"config": self[".name"], 
				"section": section[".name"]
			}).done(function(){
				_unlinkSection(self, section); 
				self[".need_commit"] = true; 
				deferred.resolve(); 
			}).fail(function(){
				console.error("Failed to delete section!"); 
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
		
		// creates a new object that will have values set to values
		UCIConfig.prototype.create = function(item, offline){
			var self = this; 
			if(!(".type" in item)) throw new Error("Missing '.type' parameter!"); 
			var type = section_types[self[".name"]][item[".type"]]; 
			if(!type) throw Error("Trying to create section of unrecognized type!"); 
			
			// TODO: validate values!
			var values = {}; 
			Object.keys(type).map(function(k){ 
				if(k in item) values[k] = item[k]; 
				else {
					if(type[k].required) throw Error("Missing required field "+k); 
					values[k] = type[k].dvalue; 
				}
			}); 
			var deferred = $.Deferred(); 
			console.log("Adding: "+item[".type"]+": "+JSON.stringify(values)); 
			$rpc.uci.add({
				"config": self[".name"], 
				"type": item[".type"],
				"name": item[".name"], 
				"values": values
			}).done(function(state){
				console.log("Added new section: "+state.section); 
				item[".name"] = state.section; 
				self[".need_commit"] = true; 
				var section = _insertSection(self, item); 
				//section[".new"] = true; 
				deferred.resolve(section); 
			}).fail(function(){
				deferred.reject(); 
			});
			return deferred.promise(); 
		}
		
		
		UCIConfig.prototype.$getWriteRequests = function(){
			var self = this; 
			var reqlist = []; 
			self["@all"].map(function(section){
				var changed = section.$getChangedValues(); 
				console.log(JSON.stringify(changed) +": "+Object.keys(changed).length); 
				if(Object.keys(changed).length){
					reqlist.push({
						"config": self[".name"], 
						"section": section[".name"], 
						"values": changed
					}); 
				}
			}); 
			return reqlist; 
		}
		
		UCI.Config = UCIConfig; 
	})(); 
	
	UCI.prototype.$init = function(){
		var deferred = $.Deferred(); 
		console.log("Init UCI"); 
		var self = this; 
		$rpc.uci.configs().done(function(response){
			var cfigs = response.configs; 
			if(!cfigs) { next("could not retrieve list of configs!"); return; }
			cfigs.map(function(k){
				if(!(k in self)){
					//console.log("Adding new config "+k); 
					self[k] = new UCI.Config(self, k); 
				}
			}); 
			deferred.resolve(); 
		}).fail(function(){
			deferred.reject(); 
		}); 
		return deferred.promise(); 
	}
	
	UCI.prototype.sync = function(configs){
		var deferred = $.Deferred(); 
		var self = this; 
		
		async.series([
			function(next){
				if(!(configs instanceof Array)) configs = [configs]; 
				if(!configs || configs.length == 0) { next(); return; }; 
				async.eachSeries(configs, function(cf, next){
					if(!(cf in self)) { 
						console.error("invalid config name "+cf); 
						next(); 
						return; 
					}; 
					self[cf].$sync().done(function(){
						console.log("Synched config "+cf); 
						
						next(); 
					}).fail(function(){
						console.error("Could not sync config "+cf); 
						next(); // continue because we want to sync as many as we can!
						//next("Could not sync config "+cf); 
					}); 
				}, function(err){
					next(err); 
				}); 
			}
		], function(err){
			setTimeout(function(){ // in case async did not defer
				if(err) deferred.reject(err); 
				else deferred.resolve(); 
			}, 0); 
		}); 
		return deferred.promise(); 
	}
	
	UCI.prototype.save = function(){
		var deferred = $.Deferred(); 
		var self = this; 
		var writes = []; 
		var add_requests = []; 
		var resync = {}; 
		
		async.series([
			function(next){ // commit configs that need committing first
				var commit_list = []; 
				Object.keys(self).map(function(k){
					if(self[k].constructor == UCI.Config){
						if(self[k][".need_commit"]) commit_list.push(self[k][".name"]); 
					}
				}); 
				async.each(commit_list, function(config, next){
					console.log("Committing changes to "+config); 
					$rpc.uci.commit({config: config}).done(function(){
						next(); 
					}).fail(function(err){
						next("could not commit config: "+err); 
					});
				}, function(){
					next(); 
				});
			}, 
			function(next){ // send all changes to the server
				Object.keys(self).map(function(k){
					if(self[k].constructor == UCI.Config){
						var reqlist = self[k].$getWriteRequests(); 
						reqlist.map(function(x){ writes.push(x); });  
					}
				}); 
				console.log("Will do following write requests: "+JSON.stringify(writes)); 
				async.eachSeries(writes, function(cmd, next){
					$rpc.uci.set(cmd).done(function(){
						console.log("Wrote config "+cmd.config); 
						resync[cmd.config] = true; 
						next(); 
					}).fail(function(){
						console.error("Failed to write config "+cmd.config); 
						next(); 
					}); 
				}, function(){
					next(); 
				}); 
			}, 
			function(next){
				async.eachSeries(Object.keys(resync), function(config, next){
					console.log("Committing changes to "+config); 
					$rpc.uci.commit({config: config}).done(function(){
						self[config][".need_commit"] = false; 
						self[config].$sync().done(function(){
							next(); 
						}).fail(function(err){
							console.log("error synching config "+config+": "+err); 
							next("syncerror"); 
						}); 
					}).fail(function(err){
						next("could not commit config: "+err); 
					}); 
				}, function(err){
					// this is to always make sure that we do this outside of this code flow
					setTimeout(function(){
						if(err) deferred.reject(err); 
						else deferred.resolve(err); 
					},0); 
				}); 
			}
		]); 
		return deferred.promise(); 
	}
	
	scope.UCI = new UCI(); 
	
	/*if(exports.JUCI){
		var JUCI = exports.JUCI; 
		JUCI.uci = exports.uci = new UCI(); 
		if(JUCI.app){
			JUCI.app.factory('$uci', function(){
				return $juci.uci; 
			}); 
		}
	}*/
})(typeof exports === 'undefined'? this : exports); 
