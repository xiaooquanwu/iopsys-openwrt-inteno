$juci.module("router")
.controller("SettingsEnergyCtrl", function($scope, $uci){
	$uci.sync(["easybox"]).done(function(){
		$scope.settings = $uci.easybox.settings; 
	}); 
}); 
