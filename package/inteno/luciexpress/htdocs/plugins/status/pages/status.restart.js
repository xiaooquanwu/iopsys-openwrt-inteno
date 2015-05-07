$juci.module("status")
.controller("StatusRestartPageCtrl", function($scope, $rpc){
	$scope.onRestart = function(){
		$rpc.luci2.system.reboot().done(function(){
			console.log("Restarting the system..."); 
		}); 
	}
}); 
