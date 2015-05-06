$juci.module("wifi")
.controller("WifiGeneralPageCtrl", function($scope, $uci, $tr){
	async.series([
		function(next){
			$uci.sync(["wireless", "easybox"]).done(function(){
				if(!$uci.wireless.status){
					$uci.wireless.create({
						".type": "wifi-status", 
						".name": "status", 
						disabled: false, 
						button_enabled: false
					}).done(function(){
						$uci.save().done(function(){
							next();
						}).fail(function(){
							throw new Error("Could not create missing wifi-settings section!"); 
						}); 
					}).always(function(){
						next(); 
					}); 
				} else {
					next(); 
				}
			}); 
		}, 
		function(next){
			window.uci = $uci; 
			$scope.interfaces = $uci.wireless['@wifi-iface']; 
			$scope.status = $uci.wireless.status; 
			$scope.easybox = $uci.easybox.settings; 
			$scope.$apply(); 
			
			next(); 
		}
	]); 
	
	$scope.onApply = function(){
		$scope.busy = 1; 
		$uci.save().done(function(){
			console.log("Settings saved!"); 
		}).fail(function(){
			console.error("Could not save wifi settings!"); 
		}).always(function(){
			$scope.busy = 0; 
		}); 
	}
}); 
