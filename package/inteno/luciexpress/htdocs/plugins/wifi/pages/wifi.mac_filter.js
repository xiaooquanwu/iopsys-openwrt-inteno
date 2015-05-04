$juci.module("wifi")
.controller("WifiMacFilterPageCtrl", function($scope, $uci, $hosts){
	window.uci = $uci; 
	$scope.uci = $uci; 
	
	$uci.sync(["wireless", "hosts"]).done(function(){
		console.log("synced wireless config"); 
		$scope.interfaces = $uci.wireless['@wifi-iface']; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
	
	$scope.onApply = function(){
		$scope.busy = 1; 
		$uci.save().done(function(){
			console.log("Saved wifi config!"); 
		}).fail(function(){
			console.log("Could not save wifi config!"); 
		}).always(function(){
			$scope.busy = 0; 
			$scope.$apply(); 
		}); 
	}
}); 
