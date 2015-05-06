$juci.module("core")
.controller("OverviewPageCtrl", function($scope, $rpc, $uci, $config, gettext, $tr){
	$scope.themeUrl = ""; 
	$scope.config = $config; 
	
	$scope.wireless = {
		clients: []
	}; 
	$scope.network = {
		clients: []
	}; 
	$scope.dsl = {
		
	}; 
	function refresh() {
		$uci.sync(["wireless", "easybox"]).done(function(){
			//var sections = result.values; 
			//var cfgs = Object.keys(sections).filter(function(x) { return x.indexOf("cfg") == 0; }); 
			//$scope.wireless.wps = cfgs.filter(function(x) { return cfgs["wps_pbc"] == 1; }).length != 0; 
			$scope.wifi = $uci.wireless; 
			$scope.easybox = $uci.easybox; 
			$scope.wifiSchedStatus = (($scope.wifi.status.schedule.value)?gettext("on"):gettext("off")); 
			$scope.wifiWPSStatus = (($scope.easybox.settings.wpsbutton.value)?gettext("on"):gettext("off")); 
			$scope.defaultHostName = $tr(gettext("Unknown")); 
			async.series([
				function(next){
					$rpc.router.clients().done(function(clients){
						//alert(JSON.stringify(Object.keys(clients).map(function(x) { return clients[x]; }))); 
						var all = Object.keys(clients).map(function(x) { return clients[x]; }); 
						$scope.wireless.clients = all.filter(function(x){
							return x.connected && x.wireless == true; 
						}); 
						$scope.network.clients = all.filter(function(x){
							return x.connected && x.wireless == false; 
						}); 
						next(); 
					}); 
				}, 
				function(next){
					$rpc.asterisk.status().done(function(data){
						if(data && data.sip){
							var accounts = []; 
							Object.keys(data.sip).map(function(k){
								if(data.sip[k].ip) accounts.push(data.sip[k]); 
							}); 
							$scope.sipAccounts = accounts; 
						}
					}).always(function(){ next(); }); 
				}, 
				function(next){
					$rpc.router.dslstats().done(function(dslstats){
						var stats = dslstats.dslstats;
						if(stats && stats.bearers && stats.bearers.length > 0){ 
							$scope.dsl.max_rate_up = Math.round(stats.bearers[0].max_rate_up / 1000); 
							$scope.dsl.max_rate_down = Math.round(stats.bearers[0].max_rate_down / 1000); 
						}
					}).always(function(){ next(); }); 
				}
			], function(){
				$scope.$apply(); 
			}); 
		}); 
	} refresh(); 
	//setInterval(refresh, 5000); 
}); 
