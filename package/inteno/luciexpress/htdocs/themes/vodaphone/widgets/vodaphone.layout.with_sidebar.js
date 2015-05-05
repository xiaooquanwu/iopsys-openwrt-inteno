
$juci.config(function($provide){
	$provide.decorator("luciFooterDirective", function($delegate){
		console.log(JSON.stringify($delegate[0])); 
		return $delegate; 
	})
}); 

$juci.module("vodaphone")
.directive("luciLayoutWithSidebar", function($http, $compile, $templateCache, $config, $provide){
	/*function deleteDirective(name){
		for(var i = 0; i < angular._invokeQueue.length; i++){
			var item = angular._invokeQueue[i];
			if(item[1] == "directive" && item.Arguments[0] == name){
				angular._invokeQueue.splice(i, 1); 
			}
		}
	}
	deleteDirective("luciLayoutWithSidebar"); 
	var plugin_root = $juci.module("vodaphone").plugin_root; 
	
	return {
		templateUrl: plugin_root + "/widgets/vodaphone.layout.with_sidebar.html", 
		transclude: true,
		controller: "luciLayoutWithSidebarController"
	}; 
	*/
	var plugin_root = $juci.module("vodaphone").plugin_root; 
	var target_tpl = "plugins/core/widgets/luci.layout.with_sidebar.html"; 
	return {
		priority: 100, // give it higher priority than built-in ng-click
		//templateUrl: plugin_root+"/widgets/vodaphone.navbar.html", 
		template: '<div ng-include="templateUrl" ng-transclude></div>',
		replace: true, 
		transclude: true, 
		link: function(scope, element, attrs){
			scope.templateUrl = plugin_root + "/widgets/vodaphone.layout.with_sidebar.html"; 
			
			if($config.theme == "vodaphone" && !$templateCache.get(plugin_root + "/widgets/vodaphone.layout.with_sidebar.html")){
				var promise = $http.get(plugin_root + "/widgets/vodaphone.layout.with_sidebar.html", {cache: $templateCache}).success(function(html) {
					$templateCache.put(target_tpl, html);
				}).then(function (response) {
					//element.replaceWith($compile($templateCache.get(target_tpl))(scope));
				});
			}
		}
	}; 
}); 
