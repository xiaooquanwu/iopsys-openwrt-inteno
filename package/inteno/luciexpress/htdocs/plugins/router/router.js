$juci.module("router")
.state("internet", {
	url: "/internet", 
	views: {
		"content": {
			templateUrl: "pages/internet.firewall.html", 
		}
	},
	onEnter: function($state){
		$juci.redirect("internet.firewall"); 
	},
})
.state("settings", {
	url: "/settings", 
	onEnter: function($state){
		$juci.redirect("settings.password"); 
	},
})
.state("status", {
	url: "/status", 
	onEnter: function($state){
		$juci.redirect("status.overview"); 
	},
}); 
