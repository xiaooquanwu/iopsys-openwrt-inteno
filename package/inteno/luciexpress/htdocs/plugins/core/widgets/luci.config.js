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
		template: '<p class="luci-config-info" ng-transclude></p>', 
		replace: true, 
		transclude: true
	 };  
})
.directive("luciConfigHeading", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<h2 ng-transclude></h2>', 
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
		template: '<tr><td class="col-xs-6"><label style="font-size: 1.2em">{{title}}</label></td><td class="col-xs-6"><div class="pull-right" ng-transclude></div></td></tr>', 
		replace: true, 
		scope: {
			title: "@"
		}, 
		transclude: true
	 };  
})
.directive("luciConfigApply", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		template: '<div class="row"><div class="btn-toolbar pull-right" >'+
			'<button class="btn btn-lg btn-primary" ng-click="onApply()" ng-disabled="busy"><i class="fa fa-spinner fa-spin" ng-show="busy"/>{{ "Apply"| translate }}</button><button class="btn btn-lg btn-default" ng-click="onCancel()">{{ "Cancel" | translate }}</button>'+
			'</div></div>', 
		replace: true, 
		controller: "luciConfigApplyController"
	 }; 
}).controller("luciConfigApplyController", function($scope, $uci){
	$scope.onApply = function(){
		$scope.busy = 1; 
		$uci.save().done(function(){
			console.log("Saved uci configuration!"); 
		}).fail(function(){
			console.error("Could not save uci configuration!"); 
		}).always(function(){
			$scope.busy = 0; 
			$scope.$apply(); 
		}); 
	}
}); 

