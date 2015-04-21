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
		rpc: {
			host: "http://192.168.1.1", 
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
				"network.interface.status", 
				"system.info"
			]
		}
	}; 
}); 
