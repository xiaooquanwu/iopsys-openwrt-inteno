$juci.module("core")
.directive("uciWirelessInterface", function($compile){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.wireless.interface.html", 
		scope: {
			interface: "=ngModel"
		}, 
		controller: "WifiInterfaceController", 
		replace: true, 
		require: "^ngModel"
	 };  
}).controller("WifiInterfaceController", function($scope){
	$scope.selectedProtection = "none"; 
	$scope.selectedRadio = "wl0"; 

	$scope.allRadios = ["wl0", "wl1", "wl0wl1"]; 
	$scope.allCryptModes = ["none", "wpa", "wpa2", "mixed-wpa", "wep-shared"]; 
	$scope.configName = ""; 
	
	// TODO: need to have generic uci variables that reflect the gui 
	$scope.$watch("interface", function(value){
		//value.disabled = (value.disabled == '1')?true:false;   
		//value.closed = (value.closed == '1')?true:false; 
		$scope.configName = $scope.interface[".name"] + ".disabled"; 
	}); 
}); 
