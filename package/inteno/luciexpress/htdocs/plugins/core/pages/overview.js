$juci.module("core")
.controller("OverviewPageCtrl", function($scope, $rpc){
	$scope.themeUrl = ""; 
	$scope.test = "Hello WOrld!"; 
	$scope.wireless = {
		clients: []
	}; 
	$scope.network = {
		clients: []
	}; 
	
	function refresh() {
		$rpc.uci.state({
			"config": "wireless"
		}).done(function(result){
			var sections = result.values; 
			var cfgs = Object.keys(sections).filter(function(x) { return x.indexOf("cfg") == 0; }); 
			$scope.wireless.wps = cfgs.filter(function(x) { return cfgs["wps_pbc"] == 1; }).length != 0; 
			$rpc.router.clients().done(function(clients){
				//alert(JSON.stringify(Object.keys(clients).map(function(x) { return clients[x]; }))); 
				var all = Object.keys(clients).map(function(x) { return clients[x]; }); 
				$scope.wireless.clients = all.filter(function(x){
					return x.connected && x.wireless == true; 
				}); 
				$scope.network.clients = all.filter(function(x){
					return x.connected && x.wireless == false; 
				}); 
				$scope.$apply(); 
			}); 
		}); 
	} refresh(); 
	setInterval(refresh, 5000); 
}); 
