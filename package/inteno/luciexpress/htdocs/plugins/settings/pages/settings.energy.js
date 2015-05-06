$juci.module("settings")
.controller("SettingsEnergyCtrl", function($scope, $uci){
	$uci.sync(["easybox"]).done(function(){
		$scope.settings = $uci.easybox.settings; 
		$scope.$apply(); 
	}); 
}); 
