$juci.module("settings")
.controller("SettingsEnergyCtrl", function($scope, $uci){
	$uci.sync(["easybox"]).done(function(){
		if($uci.easybox)
			$scope.settings = $uci.easybox.settings; 
		$scope.$apply();
	}); 
}); 
