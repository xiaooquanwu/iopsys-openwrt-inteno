$juci.module("router")
.controller("InternetExHostPageCtrl", function($scope, $rpc, $config){
	$scope.exposedHostEnabled = 0; 
	$scope.wan = {}; 
	$scope.lan = {}; 
	$scope.config = $config; 
	
	function loadDMZSettings(next){
		$rpc.uci.state({
			config: "firewall",
			section: "dmz"
		}).done(function(state){
			if(!state) {
				$scope.exposedHostEnabled = 0; 
			} else {
				$scope.exposedHostEnabled = 1; 
				$scope.lan.ip = state.values.host; 
			}
			$scope.$apply(); 
			if(next) next(); 
		}).fail(function(){ if(next) next();}); 
	}
	function saveDMZSettings(next){
		var promise = null; 
		if(!$scope.lan.ip || $scope.lan.ip == ""){
			promise = $rpc.uci.delete({
				config: "firewall", 
				section: "dmz", 
				option: "host"
			}); 
		} else {
			promise = $rpc.uci.set({
				config: "firewall",
				section: "dmz", 
				values: {
					host: $scope.lan.ip
				}
			}); 
		}
		
		promise.done(function(){
			$rpc.uci.commit({
				config: "firewall",
				section: "dmz"
			}).always(function(){ 
				if(next) next(); 
			}); 
		}).fail(function(){
			if(next) next(); 
		});  
	}
	async.parallel([
		function(next){
			$rpc.network.interface.status({
				"interface": "wan"
			}).done(function(wan){
				$scope.wan.ip = wan["ipv4-address"][0].address; 
				next(); 
			}).fail(function(){next();}); 
		}, 
		function(next){
			loadDMZSettings(next);  
		}
	], function(){
		// done
	}); 
	$scope.onEnable = function(en){
		$scope.lan.ip = ""; 
		saveDMZSettings(loadDMZSettings); 
	}; 
	$scope.onSave = function(){
		async.series([
			function(next){
				saveDMZSettings(); 
			}, 
			function(next){
				loadDMZSettings(next); 
			}
		], function(err){
			if(err) $scope.error_message = err; 
			else $scope.info_message = "Settings saved!"; 
			setTimeout(function(){ 
				$scope.error_message = null; 
				$scope.info_message = null; 
				$scope.$apply(); 
			}, 4000); 
			$scope.$apply(); 
		}); 
	}
	$scope.onReset = function(){
		loadDMZSettings(); 
	}
}); 
