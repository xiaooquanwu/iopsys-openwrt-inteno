$juci.module("core")
.directive("luciFooter", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	
	return {
		// accepted parameters for this tag
		scope: {
		}, 
		templateUrl: plugin_root+"/widgets/luci.footer.html", 
		controller: "luciFooterController"
	}; 
})
.controller("luciFooterController", function($scope, $rpc, $languages, gettextCatalog){
	// TODO: move this into a higher level controller maybe? 
	$scope.languages = $languages.getLanguages(); 
	$scope.isActiveLanguage = function(lang){
		return gettextCatalog.currentLanguage == lang.short_code; 
	}
	$scope.setLanguage = function(lang){
		$languages.setLanguage(lang.short_code); 
	}; 
	$rpc.network.interface.status({
		"interface": "wan"
	}).done(function(wan){
		$rpc.router.info().done(function(info){
			$scope.firmwareVersion = info.system.firmware; 
			$scope.wanip = wan["ipv4-address"][0].address; 
			$scope.$apply(); 
		}); 
	}); 
}); 
