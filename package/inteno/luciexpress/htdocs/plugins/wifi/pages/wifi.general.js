$juci.module("wifi")
.controller("WifiGeneralPageCtrl", function($scope){
	$scope.wifiEnabled = 1; 
	$scope.wifiButtonEnabled = 1; 
	$scope.guestWifiEnabled = 0; 
	$scope.allFreqs = [{
		label: "2.4 GHz / 5 GHz"
	}, {
		label: "2.4 GHz"
	}, {
		label: "5 GHz"
	}]; 
	$scope.allCryptModes = [{
		label: "Off"
	}, {
		label: "WPA"
	}, {
		label: "WPA2"
	}, {
		label: "WPA + WPA2"
	}, {
		label: "WEP"
	}]; 
	$scope.wifiPassword = "123123"; 
}); 
