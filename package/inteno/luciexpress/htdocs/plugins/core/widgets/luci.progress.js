$juci.module("core")
.directive("luciProgress", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		// accepted parameters for this tag
		scope: {
			percent: "=", 
			value: "=", 
			total: "=", 
			units: "="
		}, 
		templateUrl: plugin_root+"/widgets/luci.progress.html", 
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
