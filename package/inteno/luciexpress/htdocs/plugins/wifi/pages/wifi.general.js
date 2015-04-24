$juci.module("wifi")
.controller("WifiGeneralPageCtrl", function($scope, $uci, $tr){
	$scope.wifiEnabled = 1; 
	$scope.mainWifiEnabled = 1; 
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
	
	$scope.wifiCheckEnabled = function(){
		if(!$scope.radios) return 0; 
		$scope.wifiEnabled = $scope.radios
			.filter(function(x) { return x.radio == "on"; })
			.length > 0; 
		return $scope.wifiEnabled; 
	}
	
	$scope.onWifiEnable = function(){
		if(!$scope.radios) {
			$scope.wifiEnabled = !$scope.wifiEnabled; 
			return; 
		}
		var enabled = $scope.wifiEnabled; 
		if(enabled) $scope.info = $tr("STR_ENABLINGWIFIRADIOS"); 
		else $scope.info = $tr("STR_DISABLINGWIFIRADIOS"); 
		async.eachSeries($scope.radios, function(radio, next) { 
			radio.radio = ((enabled)?"on":"off"); 
			$uci.set("wireless."+radio[".name"], radio).always(function(){
				console.log("Disabling "+radio[".name"]+" "+radio.radio); 
				next(); 
			}); 
		}, function(){
			$scope.info = null; 
			$scope.$apply(); 
		}); 
	}
	$scope.onGuestEnable = function(){
		
	}
	$uci.show("wireless").done(function(interfaces){
		var list = Object.keys(interfaces)
			.map(function(x){ return interfaces[x]; });
		$scope.radios = list.filter(function(x) { return x[".name"].indexOf("wl") == 0; }); 
		$scope.virtual_radios = list.filter(function(x) { return x.device && x[".name"].indexOf("wl") != 0; }); 
		if($scope.virtual_radios.length > 0) { // TODO: get main/guest from config
			var radio = $scope.virtual_radios[0];
			$scope.main_wifi = { interface: radio, device: interfaces[radio.device] }; 
		}
		if($scope.virtual_radios.length > 1) { // TODO: get main/guest from config
			var radio = $scope.virtual_radios[1];
			$scope.guest_wifi = { interface: radio, device: interfaces[radio.device] }; 
		}
		$scope.guestWifiEnabled = $scope.guest_wifi.interface.up == '1'; 
		$scope.$apply(); 
	}); 
}); 
