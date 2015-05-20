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
.directive("luciFooter", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	
	return {
		templateUrl: plugin_root+"/widgets/luci.footer.html", 
		controller: "luciFooterController"
	}; 
})
.controller("luciFooterController", function($scope, $rpc, $config, $languages, gettextCatalog, gettext, $tr){
	// TODO: move this into a higher level controller maybe? 
	$scope.languages = $languages.getLanguages(); 
	$scope.isActiveLanguage = function(lang){
		return gettextCatalog.currentLanguage == lang.short_code; 
	}
	$scope.setLanguage = function(lang){
		$languages.setLanguage(lang.short_code); 
	}; 
	$scope.config = $config; 
	$scope.wanip = $tr(gettext("Not connected")); 
	$rpc.network.interface.dump().done(function(result){
		if(result && result.interface) {
			result.interface.map(function(i){
				if(i.interface == "wan" && i["ipv4-address"] && i["ipv4-address"].length){
					$scope.wanip = i["ipv4-address"][0].address; 
				}
			}); 
		}
		$scope.$apply(); 
	}); 
	$rpc.router.info().done(function(result){
		if(result.system) $scope.firmware = result.system.firmware; 
	}); 
}); 
