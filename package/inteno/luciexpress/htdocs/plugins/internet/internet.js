$juci.module("internet")
.state("internet", {
	url: "/internet", 
	views: {
		"content": {
			templateUrl: "pages/internet.firewall.html"
		}
	},
	onEnter: function(){
		$juci.redirect("internet.firewall"); 
	}
});

