$juci.module("core")
.directive("uciWirelessDeviceEdit", function($compile){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.wireless.device.edit.html", 
		scope: {
			device: "=ngModel"
		}, 
		controller: "WifiDeviceEditController", 
		replace: true, 
		require: "^ngModel"
	 };  
}).controller("WifiDeviceEditController", function($scope){
	$scope.allChannels = [ "auto", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ]; 
	$scope.allModes = ["802.11gn", "802.11bg", "802.11bgn", "802.11n"]; 
	$scope.allBandwidths = [ "20", "40", "2040", "80" ]; 
	
}); 
