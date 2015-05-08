$juci.module("core")
.directive("uciFirewallNatRuleEdit", function($compile, $parse){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.firewall.nat.rule.edit.html", 
		scope: {
			rule: "=ngModel"
		}, 
		controller: "uciFirewallNatRuleEdit", 
		replace: true, 
		require: "^ngModel"
	 };  
}).controller("uciFirewallNatRuleEdit", function($scope, $uci, $rpc, $log){
	$scope.portIsRange = 0;
	$scope.protocols = ["udp", "tcp"]; 
	$scope.patterns = {
		ipaddress: /^(?!0)(?!.*\.$)((1?\d?\d|25[0-5]|2[0-4]\d)(\.|$)){4}$/,
		port: /^\d{1,5}$/
	};
	$rpc.router.clients().done(function(clients){
		$scope.clients = [];
		$scope.devices = {}; 
		Object.keys(clients).map(function(x) {
			var c = clients[x]; 
			if(c.connected){
				$scope.devices[c.ipaddr] = c; 
				$scope.clients.push(c.ipaddr); 
			} 
		}); 
		$scope.$apply(); 
	});
    $scope.onAccept = function() {
        $scope.$parent.onAddRuleConfirm();
    }
}); 
