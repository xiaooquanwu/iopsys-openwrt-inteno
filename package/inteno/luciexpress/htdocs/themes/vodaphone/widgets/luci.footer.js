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
.directive("luciFooter", function(){
	var plugin_root = $juci.module("vodaphone").plugin_root; 
	
	return {
		templateUrl: plugin_root+"/widgets/luci.footer.html", 
		controller: "luciFooterController"
	}; 
})
.controller("luciFooterController", function($scope, $rpc, $config, $languages, gettextCatalog, gettext, $tr, $status){
	// TODO: move this into a higher level controller maybe? 
	$scope.languages = $languages.getLanguages(); 
	$scope.isActiveLanguage = function(lang){
		return gettextCatalog.currentLanguage == lang.short_code; 
	}
	$scope.setLanguage = function(lang){
		$languages.setLanguage(lang.short_code); 
	}; 
	$scope.config = $config; 
	setInterval(function(){
		try{ 
			$scope.wanip = $status.ubus.network.interface.wan.status["ipv4-address"][0].ipaddr; 
		} catch(e) { 
			$scope.wanip = $tr(gettext("Not connected")); 
		} 
		try{ $scope.firmware = $status.ubus.router.info.system.firmware; } catch(e) {}
	}, 1000); 
}); 