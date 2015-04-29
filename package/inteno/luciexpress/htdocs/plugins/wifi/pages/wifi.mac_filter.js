$juci.module("wifi")
.controller("WifiMacFilterPageCtrl", function($scope, $uci){
	$scope.guest_wifi = { }; 
	$scope.main_wifi = {}; 
	
	function load(){
		$uci.show("wireless").done(function(interfaces){
			var list = Object.keys(interfaces)
				.map(function(x){ return interfaces[x]; });
			$scope.devices = list.filter(function(x) { return x[".type"] == "wifi-device"; }); 
			$scope.interfaces = list.filter(function(x) { return x[".type"] == "wifi-iface"; }); 
			
			$scope.main_wifi = $scope.interfaces[0]; //$scope.interfaces.filter(function(x) { return x[".name"] == "main"; })[0] || {}; 
			$scope.guest_wifi = $scope.interfaces[1]; //$scope.interfaces.filter(function(x) { return x[".name"] == "guest"; })[0] || {}; 
			
			$scope.$apply(); 
		}); 
	} load(); 
	
	$scope.onApply = function(){
		async.series([
			function(next){
				$uci.set("wireless."+$scope.main_wifi[".name"], $scope.main_wifi).done(function(){
					
				}).always(function(){ next(); }); 
			}, 
			function(next){
				$uci.set("wireless."+$scope.guest_wifi[".name"], $scope.guest_wifi).done(function(){
					
				}).always(function(){ next(); }); 
			}
		], function(){
			$uci.commit("wireless").done(function(){
				console.log("Saved wifi settings!"); 
			}).fail(function(){
				console.log("Failed to save wifi settings!"); 
			}); 
		}); 
	}
	$scope.onCancel = function(){
		load(); 
	}
}); 
