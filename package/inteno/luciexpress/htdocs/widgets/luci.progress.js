angular.module("luci")
.directive("luciProgress", function(){
	return {
		// accepted parameters for this tag
		scope: {
			percent: "=", 
			value: "=", 
			total: "=", 
			units: "="
		}, 
		templateUrl: "widgets/luci.progress.html", 
		replace: true, 
		controller: "LuciProgressControl",
		controllerAs: "ctrl", 
		link: function(scope, element, attributes){
			// make sure we interpret the units as string
			scope.units = attributes.units; 
		}
	}; 
})
.controller("LuciProgressControl", function($scope, $navigation){
	if($scope.value && Number($scope.value) != 0)
		$scope.width = Math.round((Number($scope.value) / Number($scope.total)) * 100); 
	else
		$scope.width = 0; 
}); 
