angular.module("luci")
.controller("InitPageController", function($scope, $tr, $state, $stateParams, $config, $session, $rpc, $navigation, $location, $rootScope, $http){
	//$scope.progress = {}; 
	console.log("INIT"); 
	function progress(text, value){
		//console.log(text + ": " + value); 
		$scope.progress = {text: text, value: value};
		setTimeout(function(){
			$scope.$apply();
		}, 0);  
	} 
	async.series([
		function(next){
			//$scope.progress = { text: "test", value: 20 }; 
			progress("Getting config..", 5); 
			// TODO: use rpc
			next(); 
		},
		function(next){
			progress("Loading plugins..", 5); 
			var count = 0; 
			async.eachSeries($config.plugins, function(id, next){
				count++; 
				progress(".."+id, 10+((80/$config.plugins.length) * count)); 
				var plugin_root = "plugins/"+id; 
				$http.get(plugin_root + "/plugin.json")
				.success(function(data){
					var scripts = []; 
					data.plugin_root = plugin_root; 
					$juci.plugins[id] = data; 
					if(data && data.scripts){
						data.scripts.map(function(x){scripts.push(plugin_root + "/" + x); });
					} 
					// load page controllers
					if(data.pages) {
						Object.keys(data.pages).map(function(k){
							var page = data.pages[k]; 
							if(page.view){
								scripts.push(plugin_root + "/" + page.view); 
								$juci.$stateProvider.state(k.replace(".", "_"), {
									url: "/"+k, 
									views: {
										"content": {
											templateUrl: (page.view)?(plugin_root + "/" + page.view + ".html"):"/html/default.html", 
										}
									},
									onEnter: function($window){
										// TODO: all these redirects seem to load page multiple times. 
										//if(item.redirect) $window.location.href = "#!"+item.redirect; 
									},
									//luci_config: item
								}); 
							}
						}); 
					} 
					async.eachSeries(scripts, function(script, next){
						//console.log("...."+script); 
						progress("...."+script, 10 + ((80 / $config.plugins.length) * count)); 
						require([script], function(module){
							next(); 
						}); 
					}, function(){
						// goto next plugin
						next(); 
					}); 
				}).error(function(data){
					next(); 
				}); 
			}, function(){
				
				next(); 
			});
		}, 
		function(next){
			progress("Validating session..", 100); 
			$session.init().done(function(){
				next(); 
			}).fail(function(){
				console.log("Failed to verify session."); 
				$state.go("login"); 
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
		$juci._initialized = true; 
		
		// add this here to avoid being redirected to the 404 page from the start
		$juci.$urlRouterProvider.otherwise("/init/404"); 
		
		progress("", 100); 
		console.log("redirecting -> "+$stateParams.redirect); 
		setTimeout(function(){
			$state.go($stateParams.redirect || "overview"); 
		}, 500); 
	}); 
}); 
