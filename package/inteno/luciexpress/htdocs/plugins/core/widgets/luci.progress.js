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
 
$juci.module("core")
.directive("luciProgress", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		// accepted parameters for this tag
		scope: {
			percent: "=", 
			value: "=", 
			total: "=", 
			units: "="
		}, 
		templateUrl: plugin_root+"/widgets/luci.progress.html", 
		replace: true, 
		controller: "LuciProgressControl",
		controllerAs: "ctrl", 
		link: function(scope, element, attributes){
			// make sure we interpret the units as string
			scope.units = attributes.units; 
		}
	}; 
})
.controller("LuciProgressControl", function($scope, $navigation){
	if($scope.value && Number($scope.value) != 0)
		$scope.width = Math.round((Number($scope.value) / Number($scope.total)) * 100); 
	else
		$scope.width = 0; 
}); 
