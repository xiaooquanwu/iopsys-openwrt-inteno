$juci.module("settings")
.controller("SettingsEnergyCtrl", function($scope, $uci){
	$uci.sync(["boardpanel"]).done(function(){
		if($uci.boardpanel)
			$scope.boardpanel = $uci.boardpanel; 
		$scope.$apply();
	}); 
}); 
