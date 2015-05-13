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
		//var sections = result.values; 
		//var cfgs = Object.keys(sections).filter(function(x) { return x.indexOf("cfg") == 0; }); 
		//$scope.wireless.wps = cfgs.filter(function(x) { return cfgs["wps_pbc"] == 1; }).length != 0; 
		
		$scope.defaultHostName = $tr(gettext("Unknown")); 
		$scope.sipAccounts = []; 
		$scope.wifiSchedStatus = gettext("off"); 
		$scope.wifiWPSStatus = gettext("off"); 
		async.series([
			function(next){
				$uci.sync(["wireless", "easybox", "voice_client"]).done(function(){
					$scope.wifi = $uci.wireless; 
					$scope.easybox = $uci.easybox; 
					$scope.voice_client = $uci.voice_client; 
					if($uci.wireless && $uci.wireless.status) 
						$scope.wifiSchedStatus = (($uci.wireless.status.schedule.value)?gettext("on"):gettext("off")); 
					if($uci.wireless && $uci.wireless.wps)
						$scope.wifiWPSStatus = (($scope.easybox.settings.wpsbutton.value)?gettext("on"):gettext("off")); 
					if($uci.voice_client && $uci.voice_client["@sip_service_provider"]){
						$scope.sipAccounts = $uci.voice_client["@sip_service_provider"]; 
					}
				}).always(function(){ next(); }); 
			}, 
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
				}).fail(function(){
					next();
				});; 
			},
			function(next){
				var ifname = "wan"; // TODO: replace this with dynamic string after merge
				$rpc.network.interface.dump().done(function(interfaces){
					var conn = ""; 
					if(interfaces && interfaces.interface){
						interfaces.interface.map(function(i){
							if(i.interface == ifname){
								var dev = i.l3_device||i.device||""; 
								if(dev.indexOf("atm") == 0) conn = "ADSL"; 
								else if(dev.indexOf("ptm") == 0) conn = "VDSL"; 
								else if(dev.indexOf("eth") == 0) conn = "FTTH"; 
								else if(dev.indexOf("wwan") == 0) conn = "LTE"; 
								else if(dev.indexOf("wl") == 0) conn = "Wi-Fi"; 
								else conn = "N/A"; 
							}
						}); 
					} 
					$scope.wan_type = conn; 
				}).always(function(){ next(); }); 
			}, 
			function(next){
				/*$rpc.asterisk.status().done(function(data){
					if(data && data.sip){
						var accounts = []; 
						Object.keys(data.sip).map(function(k){
							if(data.sip[k].ip) accounts.push(data.sip[k]); 
						}); 
						$scope.sipAccounts = accounts; 
					}
				}).always(function(){ next(); }); */
				// just fall through 
				next(); 
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
	} refresh(); 
}); 
