/*
 * juci - javascript universal client interface
 *
 * Project Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 * 
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

$juci.module("vodafone")
.directive("luciNavbar", function($http, $compile, $templateCache, $config){
	var plugin_root = $juci.module("vodafone").plugin_root;
	var target_tpl = "plugins/core/widgets/luci.navbar.html"; 
	return {
    priority: 100, // give it higher priority than built-in ng-click
		//templateUrl: plugin_root+"/widgets/vodafone.navbar.html",
		replace: true, 
    link: function(scope, element, attrs){
			if($config.theme == "vodaphone" && !$templateCache.get(plugin_root + "/widgets/vodafone.navbar.html")){
				var promise = $http.get(plugin_root + "/widgets/vodafone.navbar.html", {cache: $templateCache}).success(function(html) {
					$templateCache.put(target_tpl, html);
				}).then(function (response) {
					element.replaceWith($compile($templateCache.get(target_tpl))(scope));
				});
			}
		}
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
}); 
