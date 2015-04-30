$juci.module("wifi")
.controller("WifiGeneralPageCtrl", function($scope, $uci, $tr){
	$scope.wifiEnabled = 1; 
	$scope.mainWifiEnabled = 1; 
	$scope.wifiButtonEnabled = 1; 
	$scope.guestWifiEnabled = 0; 
	$scope.guest_wifi = {}; 
	$scope.main_wifi = {}; 
	
	$scope.wifiPassword = "123123"; 
	
	$scope.wifiCheckEnabled = function(){
		if(!$scope.devices) { $scope.wifiEnabled = 0; return 0; }
		$scope.wifiEnabled = $scope.devices
			.filter(function(x) { return x.radio == "on"; })
			.length > 0; 
		return $scope.wifiEnabled; 
	}
	
	$scope.onWifiEnable = function(){
		if(!$scope.devices) {
			$scope.wifiEnabled = 0; 
			return; 
		}
		var enabled = $scope.wifiEnabled; 
		if(enabled) $scope.info = $tr("STR_ENABLINGWIFIRADIOS"); 
		else $scope.info = $tr("STR_DISABLINGWIFIRADIOS"); 
		async.eachSeries($scope.devices, function(radio, next) { 
			radio.radio = ((enabled)?"on":"off"); 
			$uci.set("wireless."+radio[".name"], radio).always(function(){
				console.log("Disabling "+radio[".name"]+" "+radio.radio); 
				next(); 
			}); 
		}, function(){
			$scope.info = null; 
			//$scope.$apply(); 
		}); 
	}
	$scope.onGuestEnable = function(){
		
	}
	$scope.onApply = function(){
		//console.log(JSON.stringify($scope.main_wifi)); 
		//console.log(JSON.stringify($scope.guest_wifi)); 
		async.series([
			function(next){
				$scope.main_wifi.commit().done(function(){
					next(); 
				}); 
				//$uci.set("wireless."+$scope.main_wifi[".name"], $scope.main_wifi).always(function(){next();}); 
			}, 
			function(next){
				$uci.set("wireless."+$scope.guest_wifi[".name"], $scope.guest_wifi).always(function(){next();}); 
			}
		], function(){
			console.log("saved wifi settings"); 
			// done
		}); 
	}
	$uci.show("wireless").done(function(interfaces){
		var list = Object.keys(interfaces)
			.map(function(x){ return interfaces[x]; });
		$scope.devices = list.filter(function(x) { return x[".type"] == "wifi-device"; }); 
		$scope.interfaces = list.filter(function(x) { return x[".type"] == "wifi-iface"; }); 
		
		// TODO: for now we use 0 and 1 but we want to be able to use config names instead!
		$scope.main_wifi = $scope.interfaces[0]; //$scope.interfaces.filter(function(x) { return x[".name"] == "main"; })[0] || {}; 
		$scope.guest_wifi = $scope.interfaces[1]; //$scope.interfaces.filter(function(x) { return x[".name"] == "guest"; })[0] || {}; 
		
		console.log(JSON.stringify($scope.main_wifi)); 
		//$scope.guestWifiEnabled = ($scope.guest_wifi && $scope.guest_wifi.up == '1'); 
		$scope.$apply(); 
	}); 
}); 
