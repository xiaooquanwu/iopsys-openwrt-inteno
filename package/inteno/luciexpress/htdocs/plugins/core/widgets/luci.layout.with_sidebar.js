$juci.module("core")
.directive("luciLayoutWithSidebar", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.layout.with_sidebar.html", 
		transclude: true,
		controller: "luciLayoutWithSidebarController",
		controllerAs: "ctrl"
	}; 
})
.controller("luciLayoutWithSidebarController", function($scope, $session){
	
}); 
