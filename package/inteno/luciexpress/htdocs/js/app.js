//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

$.jsonRPC.setup({
  endPoint: '/ubus',
  namespace: 'luci'
});

window.$ = $; 

require.config({
    baseUrl: '/',
    urlArgs: 'v=1.0'
});


Object.assign = Object.assign || function (target, source) {
	function ToObject(val) {
		if (val == null) {
			throw new TypeError('Object.assign cannot be called with null or undefined');
		}

		return Object(val);
	}

	var from;
	var keys;
	var to = ToObject(target);

	for (var s = 1; s < arguments.length; s++) {
		from = arguments[s];
		keys = Object.keys(Object(from));

		for (var i = 0; i < keys.length; i++) {
			to[keys[i]] = from[keys[i]];
		}
	}

	return to;
};

JUCI.app.config(function ($stateProvider, $locationProvider, $compileProvider, $urlRouterProvider, $controllerProvider, $provide) {
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
	}); 
	
	$urlRouterProvider.otherwise("404"); 
})
.run(function($rootScope, $state, $session, gettextCatalog, $tr, gettext, $rpc, $config, $location, $navigation){
	console.log("RUN"); 
	
	// TODO: maybe use some other way to gather errors than root scope? 
	$rootScope.errors = []; 
	$rootScope.$on("error", function(ev, data){
		$rootScope.errors.push({message: data}); 
		console.log("ERROR: "+ev.name+": "+JSON.stringify(Object.keys(ev.currentScope))); 
	}); 
	// set current language
	gettextCatalog.currentLanguage = "en"; 
	gettextCatalog.debug = true;
	
	var path = $location.path().replace(/\//g, "").replace(/\./g, "_");  
	if(!$session.isLoggedIn()){
		path = "login"; 
	}
	if(path == "") path = "overview"; 
	
	// Generate states for all loaded pages
	Object.keys($juci.plugins).map(function(pname){
		var plugin = $juci.plugins[pname]; 
		Object.keys(plugin.pages||{}).map(function(k){
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
					onEnter: function($uci, $rootScope){
						$rootScope.errors.splice(0, $rootScope.errors.length); 
						$uci.$revert(); 
						document.title = $tr(k.replace(/\//g, ".")+".title")+" - "+$tr(gettext("application.name")); 
					}
				}); 
			}
		}); 
	}); 
	
	$juci.redirect(path); 
})

// TODO: figure out how to avoid forward declarations of things we intend to override. 
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

// make autofocus directive work as expected
JUCI.app.directive('autofocus', ['$timeout', function($timeout) {
  return {
    restrict: 'A',
    link : function($scope, $element) {
      $timeout(function() {
        $element[0].focus();
      });
    }
  }
}]);

angular.element(document).ready(function() {
	JUCI.$init().done(function(){
		angular.bootstrap(document, ["luci"]);
	}).fail(function(){
		alert("JUCI failed to initialize! look in browser console for more details (this should not happen!)"); 
	}); 
});

