$juci.module("router")
.controller("InternetFirewallPageCtrl", function($scope, $uci){
	$scope.firewallSwitchState = 0; 
	$uci.sync("firewall").done(function(){
		$scope.firewall = $uci.firewall; 
		$scope.firewallSwitchState = $uci.firewall["@rule"].filter(function(rule){ return rule.enabled.value == true; }).length > 0; 
		$scope.$apply(); 
	}); 
	$scope.onFirewallToggle = function(){
		if(!$scope.firewallSwitchState) {
			$uci.firewall["@rule"].map(function(rule){
				rule.enabled.value = false; 
			}); 
		} else {
			$uci.firewall["@rule"].map(function(rule){
				rule.enabled.value = true; 
			}); 
		}
	}
}); 
