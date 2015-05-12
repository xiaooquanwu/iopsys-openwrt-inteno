$juci.app.directive("luciInputTimespan", function () {
	var plugin_root = $juci.module("core").plugin_root;
	return {
		templateUrl: plugin_root + "/widgets/luci.input.timespan.html",
		restrict: 'E',
		replace: true,
		scope: {
			model: "=ngModel"
		}, 
		controller: "luciInputTimespan"
	};
}).controller("luciInputTimespan", function($scope){
	$scope.data = {
		from: "", to: ""
	}; 
	$scope.$watch("model", function(model){
		if(!model) return; 
		var value = model.value; 
		if(!value || !value.split) return; 
		var parts = value.split("-"); 
		if(parts.length != 2){
			$scope.data.from = $scope.data.to = ""; 
		} else {
			$scope.data.from = parts[0]; 
			$scope.data.to = parts[1]; 
		}
	}); 
	$scope.$watch("data.from", function(value){
		if($scope.model) $scope.model.value = $scope.data.from + "-"+$scope.data.to; 
	}); 
	$scope.$watch("data.to", function(value){
		if($scope.model) $scope.model.value = $scope.data.from + "-"+$scope.data.to; 
	}); 
}); 
