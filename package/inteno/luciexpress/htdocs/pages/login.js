angular.module("luci")
.config(function($stateProvider, $navigationProvider){
	$stateProvider.state("login", {
		url: "/login", 
		views: {
			"content": {
				templateUrl: "/pages/login.html"
			}
		}
	}); 
})
.controller("LoginPageCtrl", function($scope){
	
}); 
