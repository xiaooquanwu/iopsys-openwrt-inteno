$juci.module("wifi")
.controller("WifiWPSPageCtrl", function($scope, $uci){
	$uci.sync(["wireless"]).done(function(){
		$scope.settings = $uci.wireless.settings; 
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
