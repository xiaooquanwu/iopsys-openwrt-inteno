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
 
$.jsonRPC.setup({
  endPoint: '/ubus',
  namespace: 'luci'
});

function supports_html5_storage() {
  try {
    return 'localStorage' in window && window['localStorage'] !== null;
  } catch (e) {
    return false;
  }
}

$juci = {
		plugins: {},
		module: function(name, root, data){
			if(data){
				data.plugin_root = root; 
				this.plugins[name] = data; 
			}
			var plugin = this.plugins[name]; 
			var juci = this; 
			return {
				plugin_root: plugin.plugin_root, 
				directive: function(name, fn){
					return juci.directive(name, fn);
				}, 
				controller: function(name, fn){
					return juci.controller(name, fn); 
				}, 
				state: function(name, obj){
					if(obj.templateUrl && plugin.plugin_root) obj.templateUrl = plugin.plugin_root + "/" + obj.templateUrl; 
					if(obj.views) Object.keys(obj.views).map(function(k){
						var v = obj.views[k]; 
						if(v.templateUrl && plugin.plugin_root) v.templateUrl = plugin.plugin_root + "/" + v.templateUrl; 
					}); 
					$juci.$stateProvider.state(name, obj); 
					return this; 
				}
			}
		}
}; 

require.config({
    baseUrl: '/',
    urlArgs: 'v=1.0'
});
/*
require([
		'pages/overview',
	], function () {
		angular.bootstrap(document, ['luci']);
});
*/
angular.module("luci", [
	"ui.bootstrap",
	"ui.router", 
	'ui.select',
	'angularModalService', 
	"uiSwitch",
	"ngAnimate", 
	"gettext"
	])
	.config(function ($stateProvider, $locationProvider, $compileProvider, $urlRouterProvider, $controllerProvider) {
		//$locationProvider.otherwise({ redirectTo: "/" });
		$locationProvider.hashPrefix('!');
		$juci.controller = $controllerProvider.register; 
		$juci.directive = $compileProvider.directive; 
		$juci.state = $stateProvider.state; 
		$juci.$stateProvider = $stateProvider; 
		$juci.$urlRouterProvider = $urlRouterProvider; 
		$juci.redirect = function(page){
			window.location.href = "#!"+page; 
		}
		
		$stateProvider.state("404", {
			url: "/404", 
			views: {
				"content": {
					templateUrl: "/html/404.html"
				}
			},
			onEnter: function(){
				if(!$juci._initialized){
					$juci.redirect("/init/404"); 
				}
			}
		}); 
		// application init state. All initialization is done here. 
		$stateProvider.state("init", {
			url: "/init/:redirect", 
			views: {
				"content": {
					templateUrl: "html/init.html"
				}
			}, 
			onEnter: function($state, $stateParams, $config, $session, $rpc, $navigation, $location, $rootScope, $http){
				if($juci._initialized) {
					$juci.redirect($stateParams.redirect || "overview"); 
					return;
				} else {
					
				}
			},
			luci_config: {}
		}); 
	})
	.run(function($rootScope, $state, $session, gettextCatalog, $rpc, $config, $location, $navigation){
		$rootScope.config = $config; 
		//$rootScope.theme_index = "html/init.html"; 
		// set current language
		//gettextCatalog.currentLanguage = "se"; 
		//gettextCatalog.debug = true;
		/*$rootScope.$on('$routeChangeSuccess', function (event, current, previous) {
        $rootScope.title = current.$$route.title;
    });*/
		var path = $location.path().replace("/", "").replace(".", "_");  
		
		$session.init().done(function(){
			$state.go("init", {"redirect": path}); 
		}).fail(function(){
			console.log("Failed to verify session."); 
			$state.go("init", {"redirect": "login"}); 
			//$state.go("login"); 
		}); 
	})

angular.module("luci")
.factory("$hosts", function($rpc, $uci){
	var hosts = {}; 
	var host_schema = schema({
		hostname: /[a-zA-Z0-9]*/,
		macaddr: /^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/
	}); 
	return {
		insert: function(obj){
			var deferred = $.Deferred(); 
			deferred.resolve(obj); 
			return deferred.promise(); 
		}, 
		select: function(rules){
			var mac = rules.macaddr; 
			var deferred = $.Deferred(); 
			if(mac in hosts) deferred.resolve(hosts[mac]); 
			else {
				async.series([
					function(next){
						$uci.show("hosts").done(function(result){
							Object.keys(result).map(function(k){
								var host = result[k]; 
								if(!host_schema(host)) {
									console.log("ERROR processing host "+k+": "+JSON.stringify(host_schema.errors(host))); 
									//return;
								}
								hosts[host.macaddr] = host; 
							}); 
							next(); 
						}).fail(function(){ next(); }); 
					}, 
					function(next){
						$rpc.router.clients().done(function(clients){
							Object.keys(clients).map(function(x){ 
								var cl = clients[x]; 
								if(!(cl.macaddr in hosts)){
									hosts[cl.macaddr] = {
										hostname: cl.hostname, 
										macaddr: cl.macaddr
									}; 
								}
							});
							next(); 
						}).fail(function(){ next(); }); 
					}
				], function(){
					console.log("HOSTS: "+JSON.stringify(hosts)); 
					if(!(mac in hosts)) deferred.reject(); 
					else deferred.resolve(hosts[mac]); 
				}); 
			}
			return deferred.promise(); 
		}
	}
}); 

$(document).ready(function(){
	          
	$("#loading-indicator").hide(); 
}); 
