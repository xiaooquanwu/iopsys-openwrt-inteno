$juci.module("wifi")
.controller("WifiWPSPageCtrl", function($scope, $uci, $rpc){
	console.log("WIFI PAGE CONTROLLER"); 
	$scope.data = {
		userPIN: ""
	}
	$scope.progress = 0; 
	
	$uci.sync(["wireless", "boardpanel"]).done(function(){
		if($uci.boardpanel == undefined) $scope.$emit("error", "Boardpanel config is not present on this system!"); 
		else $scope.boardpanel = $uci.boardpanel; 
		if(!$uci.boardpanel.settings){
			$uci.boardpanel.create({".type": "settings", ".name": "settings"}).done(function(section){
				$uci.save(); 
			}).fail(function(){
				$scope.$emit("error", "Could not create required section boardpanel.settings in config!"); 
			}); 
		} 
		$scope.wireless = $uci.wireless; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
	function retry(){
		setTimeout(function(){
			$uci.sync("broadcom").done(function(){
				$scope.progress = $uci.broadcom.nvram.wps_proc_status.value;
				$scope.$apply();	
				retry(); 
			}); 
		}, 1000); 
	} retry(); 
	
	$rpc.wps.showpin().done(function(data){
		$scope.generatedPIN = data.pin; 
	}); 
	
	$scope.save = function(){
		$uci.save(); 
	}
	$scope.onPairPBC = function(){
		$rpc.wps.pbc();
	}
	$scope.onPairUserPIN = function(){
		$rpc.wps.stapin({ pin: $scope.data.userPIN });
	}
	$scope.onGeneratePIN = function(){
		$rpc.wps.genpin().done(function(data){
			$rpc.wps.setpin({pin: data.pin}).done(function(){
				$scope.generatedPIN = data.pin; 
				$scope.$apply(); 
			}); 
		}); 
	}
}); 
