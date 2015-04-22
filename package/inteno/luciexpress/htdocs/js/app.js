/*
 * juci - javascript universal client interface
 *
 * Author: Martin K. Schröder <martin.schroder@inteno.se>
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

var PLUGIN_ROOT = ""; 

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
	"uiSwitch",
	"gettext"
	])
	.config(function ($stateProvider, $locationProvider, $urlRouterProvider) {
		//$locationProvider.otherwise({ redirectTo: "/" });
		$locationProvider.hashPrefix('!');
		//$stateProvider.otherwise("login"); 
		//$urlRouterProvider.otherwise("/otherwise"); 
		$stateProvider.state("ready", {
			url: "", 
			views: {
				"content": {
					templateUrl: "pages/overview.html"
				}
			}, 
			luci_config: {}
		}); 
		$stateProvider.state("init", {
			url: "/init", 
			views: {
				"content": {
					templateUrl: "pages/loading.html"
				}
			}, 
			onEnter: function($state, $config, $session, $rpc, $navigation, $rootScope, $http){
				console.log("INIT"); 
				async.series([
					function(next){
						console.log("Validating session.."); 
						$session.init().done(function(){
							next(); 
						}).fail(function(){
							console.log("Failed to verify session."); 
							$state.go("login"); 
						}); 
					}, 
					function(next){
						console.log("Getting config.."); 
						// TODO: use rpc
						next(); 
					},
					function(next){
						console.log("Loading plugins.."); 
						async.eachSeries($config.plugins, function(id, next){
							console.log(".."+id); 
							var plugin_root = "plugins/"+id; 
							$http.get(plugin_root + "/plugin.json")
							.success(function(data){
								if(data && data.scripts){
									var scripts = data.scripts.map(function(x){return plugin_root + "/" + x; }); 
									require(scripts, function(module){
										next(); /*
										module.plugin_init({
											PLUGIN_ROOT: root
										}, function(){
											next(); 
										}); */
									});
								} else {
									next();
								} 
							}).error(function(data){
								next(); 
							}); 
							/*var plug = $config.plugins[id]; 
							var root = "plugins/"+id+"/"; 
							require(plug.scripts.map(function(s){return "plugins/"+s+"/"+s;}), function(module){
								module.plugin_init({
									PLUGIN_ROOT: root
								}, function(){
									next(); 
								}); 
							}); */
						}, function(){
							next(); 
						});
					}, 
					function(next){
						console.log("Getting navigation.."); 
						
						// get the menu navigation
						$rpc.luci2.ui.menu().done(function(data){
							//console.log(JSON.stringify(data)); 
							Object.keys(data.menu).map(function(key){
								var menu = data.menu[key]; 
								var view = menu.view; 
								var path = key.replace("/", "."); 
								var obj = {
									path: path, 
									modes: data.menu[key].modes || [ ], 
									text: data.menu[key].title, 
									index: data.menu[key].index || 0, 
								}; 
								if(menu.redirect){
									obj.redirect = menu.redirect; 
								}
								if(view){
									obj.page = "/pages/"+view.replace("/", ".")+".html"; 
								}
								$navigation.register(obj); 
							}); 
							//$rootScope.$apply(); 
							next(); 
						}); 
					}
				], function(err){
					if(err) $state.go("error"); 
					console.log("READY"); 
					$state.go("ready"); 
				}); 
			},
			luci_config: {}
		}); 
	})
	.run(function($rootScope, $state, $session, gettextCatalog, $rpc, $navigation){
		// set current language
		//gettextCatalog.currentLanguage = "se"; 
		//gettextCatalog.debug = true; 
		$state.go("init"); 
		
	})
	
angular.module("luci").controller("BodyCtrl", function ($scope, $state, $session, $location, $window, $rootScope, $config) {
	$scope.menuClass = function(page) {
		var current = $location.path().substring(1);
		return page === current ? "active" : "";
	};
	$scope.mode = ""; 
	$scope.modeList = [{
		id: 0, 
		label: "Basic Mode"
	}]; 
	
	$config.mode = localStorage.getItem("mode") || "basic"; 
	$config.theme = localStorage.getItem("theme") || "default"; 
	
	/*setTimeout(function(){
		$("#guiMode").selectpicker('val', $config.mode || "basic"); 
	}, 100); */
	$("#guiMode").on("change", function(){
		var selected = $(this).find("option:selected").val();
		console.log(selected); 
		if(selected == "logout") {
			$session.logout().always(function(){
				$window.location.href="/"; 
			}); 
		} else {
			$config.mode = selected; 
			$state.reload(); 
		}
		localStorage.setItem("mode", selected); 
	}); 
	
})

$(document).ready(function(){
	          
	$("#loading-indicator").hide(); 
}); 
