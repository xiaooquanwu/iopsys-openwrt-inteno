angular.module("luci").config(function($stateProvider) {
	var plugin_root = $juci.module("internet").plugin_root; 
	$stateProvider.state("internet", {
		url: "/internet", 
		onEnter: function(){
			$juci.redirect("internet-firewall"); 
		}
	}); 
});
/*
$juci.module("internet")
.state("internet", {
	url: "/internet", 
	views: {
		"content": {
			templateUrl: "pages/internet.firewall.html"
		}
	},
	onEnter: function(){
		$juci.redirect("internet-firewall"); 
	}
});
*/
