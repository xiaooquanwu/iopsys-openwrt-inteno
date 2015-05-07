//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

angular.module("luci")
.controller("InitPageController", function($scope, $tr, 
	$state, $stateParams, $config, $session, $localStorage, 
	$rpc, $navigation, $location, $rootScope, $http, gettext, $theme){
	//$scope.progress = {}; 
	console.log("INIT"); 
	function progress(text, value){
		//console.log(text + ": " + value); 
		$scope.progress = {text: text, value: value};
		setTimeout(function(){
			$scope.$apply();
		}, 0);  
	} 
	
	var scripts = []; 
	async.series([
		function(next){
			//$scope.progress = { text: "test", value: 20 }; 
			progress("Getting config..", 0); 
			// TODO: use rpc
			next(); 
		},
		function(next){
			progress("Loading plugins..", 8); 
			var count = 0; 
			async.each($config.plugins, function(id, next){
				count++; 
				progress(".."+id, 10+((80/$config.plugins.length) * count)); 
				var plugin_root = "plugins/"+id; 
				$http.get(plugin_root + "/plugin.json")
				.success(function(data){
					console.log("found plugin "+id); 
					$juci.module(id, plugin_root, data); 
					if(data && data.scripts){
						data.scripts.map(function(x){scripts.push(plugin_root + "/" + x); });
					} 
					// load page controllers
					if(data.pages) {
						Object.keys(data.pages).map(function(k){
							var page = data.pages[k]; 
							if(page.view){
								//scripts.push(plugin_root + "/" + page.view); 
								//console.log("Registering state "+k.replace(/\./g, "_")); 
								// TODO: there is still a problem with state changes where template gets loaded before the dependencies so controller is not found
								$juci.$stateProvider.state(k.replace(/\./g, "_"), {
									url: "/"+k, 
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
								}); 
							}
						}); 
					} 
					next(); 
				}).error(function(data){
					next(); 
				}); 
			}, function(){
				
				next(); 
			});
		}, 
		function(next){
			// TODO: this will be moved somewhere else. What we want to do is 
			// pick both a theme and plugins based on the router model. 
			console.log("Detected hardware model: "+$config.system.hardware); 
			var themes = {
				"CG300A": "inteno-red"
			}; 
			$config.mode = $localStorage.getItem("mode") || "basic"; 
			$config.theme = $localStorage.getItem("theme") || themes[$config.system.hardware] || "inteno-red"; 
			
			//$config.theme = "default"; 
			
			$theme.changeTheme($config.theme).done(function(){
				next(); 
			}).fail(function(){
				next(); 
			}); 
		}, 
		function(next){
			async.each(scripts, function(script, next){
				//console.log("...."+script); 
				//progress("...."+script, 10 + ((80 / $config.plugins.length) * count)); 
				require([script], function(module){
					next(); 
				}); 
			}, function(){
				// goto next plugin
				next(); 
			}); 
		}, 
		function(next){
			progress("Getting navigation..", 100); 
			
			// get the menu navigation
			$rpc.luci2.ui.menu().done(function(data){
				//console.log(JSON.stringify(data)); 
				Object.keys(data.menu).map(function(key){
					var menu = data.menu[key]; 
					var view = menu.view; 
					var path = key.replace(/\//g, "."); 
					var obj = {
						path: path, 
						modes: data.menu[key].modes || [ ], 
						text: data.menu[key].title, 
						index: data.menu[key].index || 0, 
					}; 
					/*if(menu.redirect){
						obj.redirect = menu.redirect; 
					}*/
					/*if(view){
						obj.page = "/pages/"+path.replace(/\//g, ".")+".html"; 
					}*/
					$navigation.register(obj); 
					
				}); 
				//console.log("NAV: "+JSON.stringify($navigation.tree())); 
				//$rootScope.$apply(); 
				next(); 
			}).fail(function(){
				next();
			}); 
		}
	], function(err){
		if(err) {
			console.log("ERROR: "+err); 
			$state.go("error"); 
		}
		
		$juci._initialized = true; 
		
		// add this here to avoid being redirected to the 404 page from the start
		$juci.$urlRouterProvider.otherwise("/init/404"); 
		
		progress("", 100); 
		console.log("redirecting -> "+$stateParams.redirect); 
		$state.go($stateParams.redirect || "overview"); 
	}); 
}); 
