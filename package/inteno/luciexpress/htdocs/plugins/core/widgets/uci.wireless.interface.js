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
	//$scope.interface = {}; 
	$scope.selectedProtection = "none"; 
	$scope.selectedRadio = "wl0"; 
	/*$scope.allFreqs = [{
		label: "2.4 GHz / 5 GHz",
		bands: ["a", "b"]
	}, {
		label: "2.4 GHz", 
		bands: ["b"] 
	}, {
		label: "5 GHz",
		bands: ["a"]
	}]; */
	$scope.allRadios = ["wl0", "wl1", "wl0wl1"]; 
	$scope.allCryptModes = ["none", "wpa", "wpa2", "mixed-wpa", "wep-shared"]; 
	$scope.configName = ""; 
	
	// TODO: need to have generic uci variables that reflect the gui 
	$scope.$watch("interface", function(value){
		value.disabled = (value.disabled == '1')?true:false;   
		value.closed = (value.closed == '1')?true:false; 
		$scope.configName = $scope.interface[".name"] + ".disabled"; 
	
	}); 
	/*$scope.allCryptModes = [{
		label: "Off", 
		encryption: "none", 
	}, {
		label: "WPA",
		encryption: "wpa"
	}, {
		label: "WPA2",
		encryption: "wpa2"
	}, {
		label: "WPA + WPA2", 
		encryption: "mixed-wpa"
	}, {
		label: "WEP", 
		encryption: "wep-shared"
	}]; */
}); 
