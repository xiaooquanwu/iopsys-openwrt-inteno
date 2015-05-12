$juci.module("status")
.controller("StatusRestartPageCtrl", function($scope, $rpc){
	$scope.onRestart = function(){
		$scope.showConfirmation = 1; 
		/*$rpc.luci2.system.reboot().done(function(){
			console.log("Restarting the system..."); 
		}); */
	}
	
	function waitUntilDown(){
		var deferred = $.Deferred(); 
		var rpc = false; 
		var interval = setInterval(function(){
			if(!rpc){
				rpc = true; 
				$rpc.session.access().done(function(){
					
				}).fail(function(){
					clearInterval(interval); 
					deferred.resolve(); 
				}).always(function(){
					rpc = false; 
				}); 
			}
		}, 1000); 
		return deferred.promise(); 
	}
	$scope.onConfirmRestart = function(){
		$scope.showRestartProgress = 1; 
		$scope.showConfirmation = 0; 
		$scope.progress = 0; 
		$rpc.luci2.system.reboot().done(function(){
			var rpc = true; 
			$scope.message = "Waiting for reboot..."; 
			$scope.$apply(); 
			var interval = setInterval(function(){
				$scope.progress++; 
				$scope.$apply(); 
				if(!rpc){
					rpc = true; 
					$rpc.session.access().done(function(){
						$scope.showConfirmation = 0; 
						$scope.$apply(); 
						window.location.reload(); 
					}).fail(function(){
						$scope.$apply(); 
					}).always(function(){
						rpc = false; 
					}); 
				}
			}, 1000); 
			
			waitUntilDown().done(function(){
				$scope.message = "Host is rebooting..."; 
				$scope.$apply(); 
				rpc = false; 
			}); 
			console.log("Restarting the system..."); 
		});
	}
	$scope.onCancelRestart = function(){
		$scope.showConfirmation = 0; 
	}
}); 
