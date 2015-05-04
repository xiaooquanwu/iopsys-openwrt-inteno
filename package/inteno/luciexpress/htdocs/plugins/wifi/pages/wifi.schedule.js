$juci.module("wifi")
.controller("WifiSchedulePageCtrl", function($scope, $uci){
	$uci.sync(["wireless"]).done(function(){
		$scope.settings = $uci.wireless.settings; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
	
}); 
