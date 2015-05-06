$juci.module("wifi")
.controller("WifiSchedulePageCtrl", function($scope, $uci){
	$uci.sync(["wireless"]).done(function(){
		$scope.status = $uci.wireless.status; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
	
}); 
