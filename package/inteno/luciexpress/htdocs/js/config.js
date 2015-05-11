//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

(function($juci){
	function JUCIConfig(){
		this.system = {}; 
	}
	JUCIConfig.prototype.$init = function(){
		var deferred = $.Deferred(); 
		var self = this; 
		console.log("Init CONFIG"); 
		$.get("/config.json", {
			format: "json"
		}).done(function(data){
			Object.keys(data).map(function(k) { self[k] = data[k]; }); 
			deferred.resolve(); 
		}).fail(function(){
			deferred.reject(); 
		}); 
		return deferred.promise(); 
	}
	$juci.config = new JUCIConfig(); 
	
	angular.module("luci")
	.factory('$config', function(){
		return $juci.config; 
		/*{
			mode: "basic", // basic or expert supported
			//model: "Inteno DG301",
			languages: {
				"tr": {
					title: "Turkish"
				}, 
				"de": {
					title: "German"
				}, 
				"se": {
					title: "Swedish"
				},
				"en": {
					title: "English"
				}
			},
			themes: [
				//"default", 
				//"inteno-red", 
				"vodafone"
			], 
			plugins: [
				"core", 
				"phone", 
				"settings", 
				"status",
				"internet",
				//"router", 
				"wifi"
			], 
			rpc: {
				//host: "", not used anymore because we now instead do rpc forwarding in server.js!
				exposed_calls: [ // TODO: only login and access calls should always be available 
					"session.login", 
					"session.access",
					// TODO: these calls use wildcards in acl so we need to somehow get them automatically
					// for now I defined them manually here. 
					"luci2.ui.menu",
					"uci.state", 
					"uci.set", 
					"uci.add", 
					"uci.delete", 
					"uci.commit", 
					"uci.rollback",
					"uci.confirm", 
					"uci.revert", 
					"uci.changes", 
					"uci.configs",
					"wps.pbc",
					"wps.genpin", 
					"wps.setpin",
					"wps.showpin", 
					"wps.stapin", 
					"asterisk.call_log.list", 
					"asterisk.status", 
					"luci2.system.password_set", 
					"luci2.system.backup_restore", 
					"luci2.system.upgrade_test",  
					"luci2.system.upgrade_clean",  
					"luci2.system.upgrade_start", 
					"luci2.system.reset_test", 
					"luci2.system.reset_start", 
					"luci2.system.reboot", 
					"luci2.system.ping", 
					"luci2.network.ping",
					"luci2.network.traceroute", 
					"network.interface.dump", 
					"router.networks", 
					// local stuff for the node server. 
					"local.features", 
					"local.set_rpc_host"
					
				]
			}
		}; */
	}); 
})(JUCI); 
