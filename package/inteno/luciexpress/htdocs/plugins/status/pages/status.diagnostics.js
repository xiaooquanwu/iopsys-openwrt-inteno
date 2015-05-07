$juci.module("status")
.controller("StatusDiagnostics", function($scope){
	$scope.data = {}; 
	$rpc.router.networks().done(function(result){
		if(result){
			$scope.data.allInterfaces = Object.keys(result).map(function(x){return {label: x, id: x};}); 
			$scope.$apply(); 
		}
	}); 
	$scope.onTraceTest = function(){
		var item = $scope.data.traceInterface; 
		if(item && item.id){
			console.log("Performing trace on interface "+item.id); 
			$rpc.luci2.network.traceroute({ data: "google.com" }).done(function(result){
				if(result.stderr) $scope.data.traceError = result.stderr; 
				$scope.data.traceResults = result.stdout; 
				$scope.$apply(); 
			}).fail(function(error){
				$scope.data.traceResults = ""; 
				$scope.data.traceError = JSON.stringify(error); 
				$scope.$apply(); 
			}); 
		}
	}
	$scope.onPingTest = function(){
		$scope.data.pingResults = "..."; 
		$scope.data.error = "";
		$rpc.luci2.network.ping({ data: $scope.data.pingHost }).done(function(result){
			if(result.stderr) $scope.data.pingError = result.stderr; 
			$scope.data.pingResults = result.stdout; 
			$scope.$apply(); 
		}).fail(function(error){
			$scope.data.pingResults = ""; 
			$scope.data.pingError = JSON.stringify(error); 
			$scope.$apply(); 
		}); 
	}
}); 
