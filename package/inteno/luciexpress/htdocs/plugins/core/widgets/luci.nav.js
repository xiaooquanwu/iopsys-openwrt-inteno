$juci.module("core")
.directive("luciNav", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.nav.html", 
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
	
  $scope.itemVisible = function(item){
		if(!item.modes.length) return true; 
		else if(item.modes && item.modes.indexOf($config.mode) == -1) {
			return false; 
		} 
		else return true; 
	} 
	
	function activate(){
		var path = $location.path().replace(/^\/+|\/+$/g, ''); 
		var subtree = path.split(".")[0]; 
		$scope.tree = $navigation.tree(subtree);
		
		setTimeout(function(){
			$("nav ul a").removeClass("open"); 
			$("nav ul a[href='#!"+path+"']").parent().addClass("open"); 
		}, 0); 
	}; 
	$rootScope.$on('$stateChangeSuccess', function(event, toState, toParams, fromState, fromParams){
    activate(); 
  });
	activate(); 
	
	/*$scope.$on('$locationChangeSuccess', function () {
		var path = $location.path().replace(/^\/+|\/+$/g, ''); 
		var subtree = path.split(".")[0]; 
		$scope.tree = $navigation.tree(subtree); 
		$("nav ul a[href='#!"+path+"']").parent().addClass("open"); 
	}); */
}); 
