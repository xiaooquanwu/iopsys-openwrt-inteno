JUCI.app
.controller("InternetDNSPageCtrl", function ($scope, $uci, $log) {
	$scope.providers = ["dyndns.org"];
	$log.info("uci", $uci);
	$uci.sync(["network","ddns"]).done(function () {
		if ($uci.network && $uci.network.wan) {
			$scope.wan = $uci.network.wan;
			if ($scope.wan.dns){
				while($scope.wan.dns.value.length < 2) $scope.wan.dns.value.push("");
			}
		} else {
			// TODO: this should be a dynamic name (wan will not always be called wan in the future)
			$scope.$emit("Could not find WAN network on this device"); 
		}
		if ($uci.ddns && $uci.ddns["@service"]) {
			$scope.allServices = $uci.ddns["@service"].map(function(x){
				return { label: x.service_name.value, value: x };
			});  
			$scope.ddns = $uci.ddns["@service"][0];
			$scope.$apply(); 
		} else {
			$uci.ddns.create({".type": "service"}).done(function(section){
				$scope.ddns = section; 
				$scope.$apply(); 
			}); 
		}
	}); 
	
	$scope.onApply = function(){
		$uci.save(); 
	} 
});
