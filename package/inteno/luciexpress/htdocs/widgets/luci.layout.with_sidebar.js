angular.module("luci")
.directive("luciLayoutWithSidebar", function(){
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: "widgets/luci.layout.with_sidebar.html", 
		transclude: true,
		controller: "LuciLayoutWithSidebar",
		controllerAs: "ctrl"
	}; 
})
.controller("LuciLayoutWithSidebar", function($scope, $session){
	
}); 
