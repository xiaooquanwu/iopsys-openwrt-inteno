$juci.module("core")
.directive("luciLayoutNaked", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.layout.naked.html", 
		transclude: true,
		controller: "luciLayoutNakedController",
		controllerAs: "ctrl"
	}; 
})
.controller("luciLayoutNakedController", function($scope, $session){
	
}); 
