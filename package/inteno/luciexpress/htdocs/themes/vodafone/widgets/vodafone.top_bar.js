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
.directive("luciTopBar", function($http, $compile, $templateCache, $config){
	var plugin_root = $juci.module("vodafone").plugin_root;
	var target_tpl = "plugins/core/widgets/luci.top_bar.html"; 
	return {
    priority: 100, // give it higher priority than built-in ng-click
		//templateUrl: plugin_root+"/widgets/vodafone.navbar.html",
		replace: true, 
    link: function(scope, element, attrs){
			if($config.theme == "vodaphone" && !$templateCache.get(plugin_root + "/widgets/vodafone.top_bar.html")){
				var promise = $http.get(plugin_root + "/widgets/vodafone.top_bar.html", {cache: $templateCache}).success(function(html) {
					$templateCache.put(target_tpl, html);
				}).then(function (response) {
					element.replaceWith($compile($templateCache.get(target_tpl))(scope));
				});
			}
		}
	}; 
})
.controller("luciTopBarController", function($scope, $config, $session, $window, $localStorage, $state){
	$scope.model = $config.model; 
	
	$scope.guiModes = [
		{id: "basic", label: "Basic Mode"},
		{id: "expert", label: "Expert Mode"},
		{id: "logout", label: "Log out"}
	]; 
	Object.keys($scope.guiModes).map(function(k){
		var m = $scope.guiModes[k]; 
		if(m.id == $config.mode) $scope.selectedMode = m;
	});  
	$scope.onChangeMode = function(item){
		var selected = item.id; 
		if(selected == "logout") {
			$session.logout().always(function(){
				$window.location.href="/"; 
			}); 
		} else {
			$localStorage.setItem("mode", selected); 
			$config.mode = selected; 
			$state.reload(); 
		}
	};
}); */
