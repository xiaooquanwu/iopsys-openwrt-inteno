angular.module("luci").config(function($stateProvider) {
	$stateProvider.state("wifi", {
		url: "/wifi", 
		onEnter: function($state){
			$juci.redirect("wifi-general"); 
		}
	}); 
}); 
