$juci.module("core")
.directive("luciLayoutSingleColumn", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.layout.single_column.html", 
		transclude: true,
		controller: "luciLayoutSingleColumnController",
		controllerAs: "ctrl"
	}; 
})
.controller("luciLayoutSingleColumnController", function($scope, $session){
	
}); 
