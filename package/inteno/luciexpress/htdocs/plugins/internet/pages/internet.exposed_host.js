$juci.module("internet")
.controller("InternetExHostPageCtrl", function($scope, $rpc, $config, $uci, $tr){
	$scope.config = $config; 
	
	async.parallel([
		function(next){
			$uci.sync("firewall").done(function(){
				
			}).always(function(){ next(); }); 
		}, 
		function(next){
			if($uci.firewall.dmzhost == undefined){
				$uci.firewall.create({".type": "dmzhost", ".name": "dmzhost"}).done(function(){
					next(); 
				}).fail(function(){
					throw new Error("Could not create required dmzhost section in config firewall!"); 
				}); 
			} else {
				next(); 
			}
		}, 
		function(next){
			$rpc.network.interface.status({
				"interface": "wan"
			}).done(function(wan){
				if("ipv4-address" in wan)
					$scope.wan.ip = wan["ipv4-address"][0].address; 
			}).always(function(){ next(); }); 
		}
	], function(){
		$scope.firewall = $uci.firewall; 
		$scope.available = "dmz" in $uci.firewall; 
		$scope.$apply(); 
	}); 
}); 
