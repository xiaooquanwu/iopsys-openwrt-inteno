$juci.module("router")
.controller("InternetExHostPageCtrl", function($scope, $rpc){
	$scope.exposedHostEnabled = 0; 
	$scope.wan = {}; 
	$scope.lan = {}; 
	$rpc.network.interface.status({
		"interface": "wan"
	}).done(function(wan){
		$rpc.router.info().done(function(info){
			$scope.wan.ip = wan["ipv4-address"][0].address; 
			$scope.$apply(); 
		}); 
	}); 
}); 
