$juci.module("core")
.directive("luciConfig", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<div ng-transclude></div>', 
		replace: true, 
		transclude: true
	};  
})
.directive("luciConfigSection", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<div class="luci-config-section" ng-transclude></div>', 
		replace: true, 
		transclude: true
	 };  
})
.directive("luciConfigInfo", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<p class="luci-config-info" ng-transclude translate></p>', 
		replace: true, 
		transclude: true
	 };  
})
.directive("luciConfigHeading", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<h2 ng-transclude translate></h2>', 
		replace: true, 
		transclude: true
	 };  
})
.directive("luciConfigLines", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<table class="table" ><tbody ng-transclude></tbody></table>', 
		replace: true, 
		transclude: true
	 };  
})
.directive("luciConfigLine", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<tr><td class="col-xs-6"><label style="font-size: 1.2em" translate>{{title}}</label></td><td class="col-xs-6"><div class="btn-toolbar pull-right" ng-transclude></div></td></tr>', 
		replace: true, 
		scope: {
			title: "="
		}, 
		transclude: true
	 };  
})
.directive("luciConfigFooter", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<div class="row"><div class="btn-toolbar pull-right" ng-transclude></div></div>', 
		replace: true, 
		transclude: true
	 }; 
}); 

