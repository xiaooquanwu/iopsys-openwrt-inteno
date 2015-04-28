$juci.module("router")
.controller("InternetFirewallPageCtrl", function($scope, $uci){
	
	$uci.show("firewall.settings").done(function(settings){
		$scope.settings = settings; 
		settings.firewall = Number(settings.firewall); 
		settings.ping_wan = Number(settings.ping_wan); 
		
		$scope.onSave = function(){
			$uci.set("firewall.settings", $scope.settings).done(function(){
				
			}); 
		}
		$scope.$apply(); 
	}); 
}); 
