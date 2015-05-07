$juci.module("internet")
.controller("InternetExHostPageCtrl", function($scope, $rpc, $config, $uci, $tr){
	$scope.exposedHostEnabled = 0; 
	$scope.wan = {}; 
	$scope.lan = {}; 
	$scope.config = $config; 
	
	async.parallel([
		function(next){
			$uci.sync("firewall").done(function(){
				$scope.firewall = $uci.firewall; 
			}).always(function(){ next(); }); 
		}, 
		function(next){
			$rpc.network.interface.status({
				"interface": "wan"
			}).done(function(wan){
				$scope.wan.ip = wan["ipv4-address"][0].address; 
			}).always(function(){ next(); }); 
		}
	]); 
	
	$scope.onEnable = function(en){
		$scope.dmz.enabled = $scope.exposedHostEnabled; 
		$uci.set("firewall.dmz", $scope.dmz).done(function(){
			
		}); 
	}; 
	$scope.onSave = function(){
		$uci.set("firewall.dmz", $scope.dmz).done(function(){
			$uci.commit("firewall.dmz").done(function(){
				$scope.info_message = $tr("STR_SETTINGSSAVED"); 
				$scope.$apply(); 
			}); 
		}).fail(function(){ 
			$scope.error_message = $tr("STR_SETTINGSSAVEFAILED"); 
		}); 
		setTimeout(function(){ 
			$scope.error_message = null; 
			$scope.info_message = null; 
			$scope.$apply(); 
		}, 4000); 
	}
	
	$scope.onReset = function(){
		load(); 
	}
}); 
