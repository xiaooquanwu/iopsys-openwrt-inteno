//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

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

window.$ = $; 
var JUCI = {}; 
var $juci = JUCI; 
(function(){
	function JUCIMain(){
		this.plugins = {}; 
	}
	
	JUCIMain.prototype.module = function(name, root, data){
		if(data){
			data.plugin_root = root; 
			this.plugins[name] = data; 
		}
		var plugin = this.plugins[name]; 
		var juci = this; 
		return {
			plugin_root: plugin.plugin_root, 
			directive: function(name, fn){
				return angular.module("luci").directive(name, fn);
			}, 
			controller: function(name, fn){
				return angular.module("luci").controller(name, fn); 
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
	}; 
	JUCI = $juci = window.juci = new JUCIMain(); 
})(); 

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
	"gettext", 
	"checklist-model"
]); 

angular.module("luci")
.config(function ($stateProvider, $locationProvider, $compileProvider, $urlRouterProvider, $controllerProvider, $provide) {
	console.log("CONF"); 
	//$locationProvider.otherwise({ redirectTo: "/" });
	$locationProvider.hashPrefix('!');
	$locationProvider.html5Mode(false); 
	$juci.controller = $controllerProvider.register; 
	$juci.directive = $compileProvider.directive; 
	$juci.state = $stateProvider.state; 
	$juci.decorator = function(name, func){
		return $provide.decorator(name, func); 
	}
	$juci.$config = angular.module("luci").config; 
	$juci.$stateProvider = $stateProvider; 
	
	
	/*$provide.decorator("luciFooterDirective", function($delegate){
		console.log(JSON.stringify($delegate[0])); 
		return $delegate; 
	});*/ 

	$juci.$urlRouterProvider = $urlRouterProvider; 
	$juci.redirect = function(page){
		window.location.href = "#!/"+page; 
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
.run(function($rootScope, $state, $session, gettextCatalog, $tr, gettext, $rpc, $config, $location, $navigation){
	console.log("RUN"); 
	
	$rootScope.config = $config; 
	//window.rpc = $rpc; 
	//window.uci = $uci; 
	
	// set current language
	gettextCatalog.currentLanguage = "se"; 
	gettextCatalog.debug = true;
	/*$rootScope.$on('$routeChangeSuccess', function (event, current, previous) {
			$rootScope.title = current.$$route.title;
	});*/
	var path = $location.path().replace(/\//g, "").replace(/\./g, "_");  
	
	Object.keys($juci.plugins).map(function(pname){
		var plugin = $juci.plugins[pname]; 
		Object.keys(plugin.pages).map(function(k){
			var page = plugin.pages[k]; 
			if(page.view){
				//scripts.push(plugin_root + "/" + page.view); 
				var url = k.replace(/\./g, "-").replace(/_/g, "-").replace(/\//g, "-"); 
				var name = url.replace(/\//g, "_").replace(/-/g, "_"); 
				//console.log("Registering state "+name+" at "+url); 
				var plugin_root = "/plugins/"+pname; 
				$juci.$stateProvider.state(name, {
					url: "/"+url, 
					views: {
						"content": {
							templateUrl: plugin_root + "/" + page.view + ".html", 
						}
					},
					// Perfect! This loads our controllers on demand! :) 
					/*resolve: {
						deps : function ($q, $rootScope) {
							var deferred = $q.defer();
							require([plugin_root + "/" + page.view + ".js"], function (tt) {
								$rootScope.$apply(function () {
										deferred.resolve();
								});
								deferred.resolve()
							});
							return deferred.promise;
						}
					},*/
					onEnter: function($window){
						document.title = $tr(k+".title")+" - "+$tr(gettext("application.name")); 
						// TODO: all these redirects seem to load page multiple times. 
						//if(item.redirect) $window.location.href = "#!"+item.redirect; 
					},
					//luci_config: item
				}); 
			}
		}); 
	}); 
	$config.system = {}; 
	$session.init().done(function(){
		// here we get router info part of the config. It will allow us to 
		// pick the correct theme in the init script. TODO: perhaps do this somewhere else? 
		$rpc.router.info().done(function(info){
			//console.log("Router info: "+JSON.stringify(info.system)); 
			if(info && info.system) $config.system = info.system; 
			$state.go("init", {"redirect": path}); 
		}).fail(function(){
			console.error("Could not get system info. This gui depends on questd. You likely do not have it installed on your system!"); 
			$state.go("init", {"redirect": path}); 
		});
	}).fail(function(){
		console.log("Failed to verify session."); 
		$state.go("init", {"redirect": "login"}); 
		//$state.go("login"); 
	}); 
})
	
.directive("luciFooter", function(){ return {} })
.directive("luciLayoutNaked", function(){ return {} })
.directive("luciLayoutSingleColumn", function(){ return {} })
.directive("luciLayoutWithSidebar", function(){ return {} })
.directive("luciNav", function(){ return {} })
.directive("luciNavbar", function(){ return {} })
.directive("luciTopBar", function(){ return {} })
.directive('ngOnload', [function(){
	return {
		scope: {
				callBack: '&ngOnload'
		},
		link: function(scope, element, attrs){
				element.on('load', function(){
						return scope.callBack();
				})
		}
}}]); 

angular.element(document).ready(function() {
	var scripts = []; 
	async.series([
		function(next){
			$juci.ubus.$init().fail(function(){
				console.error("JUCI ubus failed to initialize!"); 
			}).always(function(){ next(); }); 
		}, 
		function(next){
			$juci.config.$init().fail(function(){
				console.error("JUCI config failed to initialize!"); 
			}).always(function(){ next(); }); 
		}, 
		function(next){
			var count = 0; 
			var scripts = []; 
			async.each($juci.config.plugins, function(id, next){
				count++; 
				console.log(".."+id, 10+((80/$juci.config.plugins.length) * count)); 
				var plugin_root = "plugins/"+id; 
				$.getJSON(plugin_root + "/plugin.json")
				.done(function(data){
					console.log("found plugin "+id); 
					$juci.module(id, plugin_root, data); 
					$juci.plugins[id] = data; 
					if(data && data.scripts){
						data.scripts.map(function(x){scripts.push(plugin_root + "/" + x); });
					} 
					// load page controllers
					if(data.pages) {
						Object.keys(data.pages).map(function(k){
							var page = data.pages[k]; 
							if(page.view){
								//scripts.push(plugin_root + "/" + page.view); 
								var url = k.replace(/\./g, "-").replace(/_/g, "-").replace(/\//g, "-"); 
								var name = url.replace(/\//g, "_").replace(/-/g, "_"); 
								//console.log("Registering state "+name+" at "+url); 
								scripts.push(plugin_root + "/" + page.view); 
								// TODO: there is still a problem with state changes where template gets loaded before the dependencies so controller is not found
								/*$juci.$stateProvider.state(name, {
									url: "/"+url, 
									views: {
										"content": {
											templateUrl: plugin_root + "/" + page.view + ".html", 
										}
									},
									// Perfect! This loads our controllers on demand! :) 
									resolve: {
											deps : function ($q, $rootScope) {
													var deferred = $q.defer();
													require([plugin_root + "/" + page.view], function (tt) {
															$rootScope.$apply(function () {
																	deferred.resolve();
															});
															deferred.resolve()
													});
													return deferred.promise;
											}
									},
									onEnter: function($window){
										document.title = $tr(k+".title")+" - "+$tr(gettext("application.name")); 
										// TODO: all these redirects seem to load page multiple times. 
										//if(item.redirect) $window.location.href = "#!"+item.redirect; 
									},
									//luci_config: item
								}); */
							}
						}); 
					} 
					next(); 
				}).error(function(data){
					next(); 
				}); 
			}, function(){
				next(); 
				/*async.each(scripts, function(script, next){
					console.log("...."+script); 
					//progress("...."+script, 10 + ((80 / $config.plugins.length) * count)); 
					require([script], function(module){
						next(); 
					}); 
				}, function(){
					// goto next plugin
					next(); 
				}); */
			});
		}, 
		function(next){
			// TODO: this will be moved somewhere else. What we want to do is 
			// pick both a theme and plugins based on the router model. 
			//console.log("Detected hardware model: "+$juci.config.system.hardware); 
			var themes = {
				"CG300A": "inteno-red"
			}; 
			var $config = $juci.config; 
			
			$config.mode = $juci.localStorage.getItem("mode") || "basic"; 
			$config.theme = $juci.localStorage.getItem("theme") || themes[$config.system.hardware] || "inteno-red"; 
			
			$config.theme = "vodafone";
			
			$juci.theme.changeTheme($config.theme).done(function(){
				next(); 
			}).fail(function(){
				next(); 
			}); 
		}, 
		function(next){
			require(scripts, function(module){
				next(); 
			}); 
		}
	], function(){
		angular.bootstrap(document, ["luci"]);
	}); 
});

/*
angular.module("luci")
.factory("$hosts", function($rpc, $uci){
	var hosts = {}; 
	return {
		insert: function(host){
			var deferred = $.Deferred(); 
			if(host.macaddr in hosts){
				deferred.resolve(hosts[host.macaddr]); 
			} else {
				$uci.add("hosts", "host", host).done(function(id){
					$uci.commit("hosts").done(function(){
						console.log("Added new host "+host.macaddr+" to database: "+id); 
						hosts[host.macaddr] = host; 
						deferred.resolve(host); 
					}).fail(function(){deferred.reject();}); 
				}).fail(function(){
					deferred.reject(); 
				});
			}
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
								hosts[result[k].macaddr] = result[k]; 
							}); 
							if(mac in hosts) next(hosts[mac]); 
							else next(); 
						}).fail(function(){ next(); }); 
					}, 
					function(next){
						$rpc.router.clients().done(function(clients){
							async.eachSeries(Object.keys(clients), function(x, next){
								var cl = clients[x]; 
								if(!(cl.macaddr in hosts)){
									console.log("Adding host "+cl.macaddr); 
									
									var host = {
										hostname: cl.hostname, 
										macaddr: cl.macaddr
									}; 
									$uci.add("hosts", "host", host).done(function(id){
										$uci.commit("hosts").done(function(){
											console.log("Added new host "+cl.macaddr+" to database - "+id); 
											hosts[cl.macaddr] = host; 
										}).always(function(){ next(); }); 
									});
								} else {
									next(); 
								}
							}, function(){
								next(); 
							}); 
						}).fail(function(){ next(); }); 
					}
				], function(){
					console.log("HOSTS: "+JSON.stringify(hosts)); 
					if(!(mac in hosts)) deferred.reject(); 
					else deferred.resolve(hosts[mac]); 
				}); 
			}
			return deferred.promise(); 
		},
		commit: function(){
			var deferred = $.Deferred(); 
			async.eachSeries(Object.keys(hosts), function(x, next){
				var h = hosts[x]; 
				if(!h || !h.commit) {
					console.log("Could not commit host "+x); 
					next(); 
				} else {
					h.commit().always(function(){next(); }); 
				}
			}, function(){
				deferred.resolve(); 
			}); 
			return deferred.promise(); 
		}
	}
}); 

$(document).ready(function(){
	          
	$("#loading-indicator").hide(); 
}); */
