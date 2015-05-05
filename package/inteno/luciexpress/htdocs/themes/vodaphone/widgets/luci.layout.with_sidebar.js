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
 
$juci.module("vodaphone")
.directive("luciLayoutWithSidebar", function(){
	var plugin_root = $juci.module("vodaphone").plugin_root; 
	
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.layout.with_sidebar.html", 
		transclude: true,
		controller: "luciLayoutWithSidebarController",
		controllerAs: "ctrl"
	}; 
})
.controller("luciLayoutWithSidebarController", function($scope, $session){
	
}); 