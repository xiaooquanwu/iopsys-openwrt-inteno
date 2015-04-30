$juci.module("wifi")
.controller("WifiSettingsPageCtrl", function($scope, $uci){
	function load(){
		$uci.show("wireless").done(function(interfaces){
			var list = Object.keys(interfaces)
				.map(function(x){ return interfaces[x]; });
			$scope.devices = list.filter(function(x) { return x[".type"] == "wifi-device"; }); 
			$scope.interfaces = list.filter(function(x) { return x[".type"] == "wifi-iface"; }); 
			
			$scope.wifi24 = $scope.devices[0]; //$scope.interfaces.filter(function(x) { return x[".name"] == "main"; })[0] || {}; 
			$scope.wifi5 = $scope.devices[1]; //$scope.interfaces.filter(function(x) { return x[".name"] == "guest"; })[0] || {}; 
			
			$scope.$apply(); 
		}); 
	} load(); 
	
	
	$scope.onApply = function(){
		$scope.busy = 1; 
		async.series([
			function(next){
				console.log("Saving object: "+JSON.stringify($scope.wifi24)); 
				$uci.set("wireless."+$scope.wifi24[".name"], $scope.wifi24).done(function(){
					
				}).always(function(){ next(); }); 
			}, 
			function(next){
				console.log("Saving object: "+JSON.stringify($scope.wifi5)); 
				$uci.set("wireless."+$scope.wifi5[".name"], $scope.wifi5).done(function(){
					
				}).always(function(){ next(); }); 
			}
		], function(){
			$uci.commit("wireless").done(function(){
				console.log("Saved wifi settings!"); 
			}).fail(function(){
				console.log("Failed to save wifi settings!"); 
			}).always(function(){
				$scope.busy = 0;
				$scope.$apply(); 
			});  
		}); 
	}
}); 
