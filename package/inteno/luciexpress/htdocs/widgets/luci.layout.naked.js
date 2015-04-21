angular.module("luci")
.directive("luciLayoutNaked", function(){
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: "widgets/luci.layout.naked.html", 
		transclude: true,
		controller: "LuciLayoutNaked",
		controllerAs: "ctrl"
	}; 
})
.controller("LuciLayoutNaked", function($scope, $session){
	
}); 
