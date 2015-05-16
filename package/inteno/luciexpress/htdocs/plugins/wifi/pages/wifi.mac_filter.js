$juci.module("wifi")
.controller("WifiMacFilterPageCtrl", function($scope, $uci){
	window.uci = $uci; 
	$scope.uci = $uci; 
	
	$uci.sync(["wireless", "hosts"]).done(function(){
		console.log("synced wireless config"); 
		$scope.interfaces = $uci.wireless['@wifi-iface']; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
}); 
