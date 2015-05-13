$juci.module("wifi")
.controller("WifiWPSPageCtrl", function($scope, $uci, $rpc){
	console.log("WIFI PAGE CONTROLLER"); 
	$scope.data = {
		userPIN: ""
	}
	$uci.sync(["wireless", "easybox"]).done(function(){
		if($uci.easybox == undefined) $scope.$emit("error", "Easybox config is not present on this system!"); 
		else $scope.easybox = $uci.easybox; 
		$scope.wireless = $uci.wireless; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	$rpc.wps.showpin().done(function(data){
		$scope.generatedPIN = data.pin; 
	}); 
	function pair(){
		$scope.showProgress = 1; 
		$scope.pairState = 'progress';
		$scope.timeRemaining = 60;  
		$uci.save().done(function(){
			var interval = setInterval(function(){
				$scope.timeRemaining --; 
				if($scope.timeRemaining == 0){
					clearInterval(interval); 
				}
			}, 1000); 
			$rpc.wps.pbc().done(function(){
				clearInterval(interval); 
				$scope.pairState = 'success'; 
				$scope.$apply(); 
				setTimeout(function(){
					$scope.showProgress = 0; 
					$scope.$apply(); 
				}, 2000); 
			}).fail(function(){
				clearInterval(interval); 
				$scope.pairState = 'fail'; 
				$scope.$apply(); 
				setTimeout(function(){
					$scope.showProgress = 0; 
					$scope.$apply(); 
				}, 2000); 
			}).always(function(){
				$scope.showProgress = 0; 
			}); 
		}); 
	}
	$scope.save = function(){
		$uci.save(); 
	}
	$scope.onPairPBC = function(){
		
		pair(); 
	}
	$scope.onPairUserPIN = function(){
		$rpc.wps.stapin({ pin: $scope.data.userPIN }).done(function(data){
			pair(); 
		}); 
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
