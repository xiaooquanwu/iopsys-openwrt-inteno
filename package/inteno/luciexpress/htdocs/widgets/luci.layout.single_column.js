angular.module("luci")
.directive("luciLayoutSingleColumn", function(){
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: "widgets/luci.layout.single_column.html", 
		transclude: true,
		controller: "LuciLayoutSingleColumn",
		controllerAs: "ctrl"
	}; 
})
.controller("LuciLayoutSingleColumn", function($scope, $session){
	
}); 
