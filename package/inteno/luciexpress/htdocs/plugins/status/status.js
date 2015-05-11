angular.module("luci").config(function($stateProvider) {
	$stateProvider.state("status", {
		url: "/status", 
		onEnter: function($state){
			$juci.redirect("status-status"); 
		},
	}); 
}); 
