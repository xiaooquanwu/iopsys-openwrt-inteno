JUCI.app
.directive("uciNetworkInterfaceEdit", function($compile, $parse){
	var plugin_root = $juci.module("internet").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.network.interface.edit.html", 
		scope: {
			interface: "=ngModel"
		}, 
		controller: "uciNetworkInterfaceEdit", 
		replace: true, 
		require: "^ngModel"
	 };  
}).controller("uciNetworkInterfaceEdit", function($scope, $uci, $rpc, $log){
	$scope.$watch("interface", function(interface){
		
	}); 
}); 
