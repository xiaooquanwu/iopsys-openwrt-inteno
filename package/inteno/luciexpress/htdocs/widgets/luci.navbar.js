
angular.module("luci")
.directive("luciNavbar", function(){
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: "widgets/luci.navbar.html", 
		replace: true, 
		controller: "NavigationCtrl",
		controllerAs: "ctrl"
	}; 
})
.controller("NavigationCtrl", function($scope, $location, $navigation, $rootScope, $config, $rpc){
	$scope.tree = $navigation.tree(); 
	
	$scope.hasChildren = function(menu){
		return menu.children_list > 0; 
	}
	$scope.isActive = function (viewLocation) { 
		return viewLocation === $location.path();
	};
	$(function(){
		var themes = $config.themes; 
		$config.theme = localStorage.getItem("theme") || "default"; 
		var bootstrap = $('<link href="'+themes[$config.theme]+'/css/bootstrap.min.css" rel="stylesheet" />');
		var theme = $('<link href="'+themes[$config.theme]+'/css/theme.css" rel="stylesheet" />');
		bootstrap.appendTo('head');
		theme.appendTo('head'); 
		$('.theme-link').click(function(){
			var themename = $(this).attr('data-theme'); 
			var themeurl = themes[themename]; 
			$config.theme = themename; 
			localStorage.setItem("theme", themename); 
			bootstrap.attr('href',themeurl+"/css/bootstrap.min.css");
			theme.attr('href',themeurl+"/css/theme.css");
		});
	});
	
	function activate(){
		var path = $location.path().replace(/^\/+|\/+$/g, ''); 
		var subtree = path.split(".")[0]; 
		$scope.tree = $navigation.tree();
		
		setTimeout(function(){
			
			$("ul.nav li a").parent().removeClass("open"); 
			$("ul.nav li a[href='#!"+subtree+"']").addClass("open"); 
			$("ul.nav li a[href='#!"+subtree+"']").parent().addClass("open"); 
		}, 0); 
	}; 
	$rootScope.$on('$stateChangeSuccess', function(event, toState, toParams, fromState, fromParams){
		activate(); 
  });
	activate(); 
}); 
/*
angular.module('autoActive', [])
		.directive('autoActive', ['$location', function ($location) {
		return {
				restrict: 'A',
				scope: false,
				link: function (scope, element) {
						function setActive() {
								var path = $location.path();
								if (path) {
										angular.forEach(element.find('li'), function (li) {
												var anchor = li.querySelector('a');
												if (anchor.href.match('#' + path + '(?=\\?|$)')) {
														angular.element(li).addClass('active');
												} else {
														angular.element(li).removeClass('active');
												}
										});
								}
						}

						setActive();

						scope.$on('$locationChangeSuccess', setActive);
				}
		}
}]);*/

