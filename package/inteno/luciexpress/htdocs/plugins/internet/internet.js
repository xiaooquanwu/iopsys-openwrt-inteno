angular.module("luci").config(function($stateProvider) {
	$stateProvider.state("internet", {
		url: "/internet", 
		views: {
			"content": {
				templateUrl: "/plugins/internet/pages/internet.firewall.html"
			}
		},
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
