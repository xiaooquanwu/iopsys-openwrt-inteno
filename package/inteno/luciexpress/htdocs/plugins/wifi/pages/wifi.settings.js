$juci.module("wifi")
.controller("WifiSettingsPageCtrl", function($scope, $uci){
	function load(){
		$uci.sync("wireless").done(function(){
			$scope.devices = $uci.wireless["@wifi-device"]; 
			$scope.$apply(); 
		}); 
	} load(); 
	
	
	$scope.onApply = function(){
		$scope.busy = 1; 
		$uci.save().done(function(){
			console.log("Saved configuration!"); 
		}).always(function(){
			$scope.busy = 0; 
			$scope.$apply(); 
		}); 
	}
}); 
