$juci.module("wifi")
.controller("WifiWPSPageCtrl", function($scope, $uci){
	$uci.sync(["easybox"]).done(function(){
		$scope.easybox = $uci.easybox; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 

}); 
