
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
.controller("NavigationCtrl", function($scope, $navigation, $config, $rpc){
	$scope.tree = $navigation.tree(); 
	/*$rpc.luci2.ui.menu().done(function(data){
		console.log(JSON.stringify(data)); 
		var tree = {children_list: []}; 
		Object.keys(data.menu).map(function(key){
			var parts = key.split("/"); 
			var obj = tree; 
			var parent = tree; 
			var insert = {
				path: (data.menu[key].view || "").replace("/", "."), 
				text: data.menu[key].title,
				children_list: []
			}; 
			// find the leaf and the parent of the leaf
			parts.map(function(part){
				if(obj.hasOwnProperty(part)) {
					parent = obj; 
					obj = obj[part]; 
				} else {
					obj[part] = {children_list: []}; 
					parent = obj; 
					obj = obj[part]; 
				}
			}); 
			Object.assign(obj, insert); 
			parent.children_list.push(obj); 
		}); 
		console.log(); 
		$scope.tree = tree; 
		$scope.$apply(); 
	}); */
	$scope.hasChildren = function(menu){
		return menu.children_list > 0; 
	}
	$scope.isActive = function (viewLocation) { 
		return viewLocation === $location.path();
	};
	$(function(){
		var themes = $config.themes; 
		var bootstrap = $('<link href="'+themes['default']+'/css/bootstrap.min.css" rel="stylesheet" />');
		var theme = $('<link href="'+themes['default']+'/css/theme.css" rel="stylesheet" />');
		bootstrap.appendTo('head');
		theme.appendTo('head'); 
		$('.theme-link').click(function(){
			var themeurl = themes[$(this).attr('data-theme')]; 
			bootstrap.attr('href',themeurl+"/css/bootstrap.min.css");
			theme.attr('href',themeurl+"/css/theme.css");
		});
	});
}); 

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
}]);

