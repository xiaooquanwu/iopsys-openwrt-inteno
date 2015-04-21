angular.module("luci")
.directive("luciNav", function(){
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: "widgets/luci.nav.html", 
		replace: true, 
		controller: "NavCtrl",
		controllerAs: "ctrl"
	}; 
})
.controller("NavCtrl", function($scope, $navigation, $location, $state, $rootScope, $config){
	var path = $location.path().replace(/^\/+|\/+$/g, ''); 
	var subtree = path.split(".")[0]; 
	
	$scope.tree = $navigation.tree(subtree); 
	$scope.hasChildren = function(menu){
		return Object.keys(menu.children) > 0; 
	}
	$scope.isActive = function (viewLocation) { 
		return viewLocation === $location.path();
	};
	$scope.$on('$stateChangeSuccess', function(event, toState, toParams, fromState, fromParams){
    var path = $location.path().replace(/^\/+|\/+$/g, ''); 
		var subtree = path.split(".")[0]; 
		$scope.tree = $navigation.tree(subtree);
		// todo: this does not seem right 
		setTimeout(function(){
			$("nav ul a").removeClass("open"); 
			$("nav ul a[href='#!"+path+"']").parent().addClass("open"); 
		}, 0); 
  });
  $scope.itemVisible = function(item){
		if(!item.modes.length) return true; 
		else if(item.modes && item.modes.indexOf($config.mode) == -1) {
			return false; 
		} 
		else return true; 
	} 
	/*$scope.$on('$locationChangeSuccess', function () {
		var path = $location.path().replace(/^\/+|\/+$/g, ''); 
		var subtree = path.split(".")[0]; 
		$scope.tree = $navigation.tree(subtree); 
		$("nav ul a[href='#!"+path+"']").parent().addClass("open"); 
	}); */
}); 
