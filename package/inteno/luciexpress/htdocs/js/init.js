angular.module("luci")
.controller("InitPageController", function($scope, $tr, $state, $stateParams, $config, $session, $localStorage, $rpc, $navigation, $location, $rootScope, $http, $theme){
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
			$config.mode = $localStorage.getItem("mode") || "basic"; 
			$config.theme = $localStorage.getItem("theme") || $config.theme || "default"; 
			
			//$config.theme = "default"; 
			
			$theme.changeTheme($config.theme).done(function(){
				next(); 
			}).fail(function(){
				next(); 
			}); 
			/*
			if($config.themes){
				var themes = {}; 
				async.eachSeries($config.themes, function(theme_id, next){
					console.log("Loading theme "+theme_id); 
					var theme_root = "themes/"+theme_id; 
					$http.get(theme_root+"/theme.json").success(function(data){
						if(!data) return; 
						Object.keys(data).map(function(k){
							themes[k] = data[k]; 
							var t = data[k]; 
							if(t.scripts) scripts = scripts.concat(t.scripts.map(function(x){return theme_root + "/"+x;})); 
							//alert(JSON.stringify(data)); 
						}); 
						$juci.module(theme_id, theme_root, data); 
						next(); 
					}).error(function(){
						next(); 
					}); 
				}, function(){
					if($config.theme in themes){
						console.log("Using theme "+$config.theme); 
						var th = "themes/"+$config.theme; 
						$rootScope.theme_index = th+"/index.html"; 
						var theme = $('<link href="'+th+'/css/theme.css" rel="stylesheet" />');
						theme.appendTo('head'); 
					} else {
						console.log("Could not load theme "+$config.theme+"!"); 
						var th = themes["default"]; 
						$rootScope.theme_index = th+"/index.html"; 
						var theme = $('<link href="'+th+'/css/theme.css" rel="stylesheet" />');
						theme.appendTo('head'); 
					} 
					
					next(); 
				}); 
			} else {
				alert("You have no themes defined in config file!"); 
				next(); 
			}*/
		}, 
		function(next){
			progress("Loading plugins..", 8); 
			var count = 0; 
			async.eachSeries($config.plugins, function(id, next){
				count++; 
				progress(".."+id, 10+((80/$config.plugins.length) * count)); 
				var plugin_root = "plugins/"+id; 
				$http.get(plugin_root + "/plugin.json")
				.success(function(data){
					$juci.module(id, plugin_root, data); 
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
		setTimeout(function(){
			$state.go($stateParams.redirect || "overview"); 
		}, 500); 
	}); 
}); 
