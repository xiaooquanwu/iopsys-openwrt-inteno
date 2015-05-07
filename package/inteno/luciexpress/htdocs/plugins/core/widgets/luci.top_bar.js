$juci.module("core")
.directive("luciTopBar", function($compile){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/luci.top_bar.html", 
		controller: "luciTopBarController", 
		replace: true
	 };  
})
.controller("luciTopBarController", function($scope, $config, $session, $uci, $window, $localStorage, $state, gettext){
	$uci.sync("system").done(function(){
		if($uci.system["@system"] && $uci.system["@system"].length){
			$scope.model = $uci.system["@system"][0].displayname.value; 
		} else {
			$scope.model = ($config.system.name || "") + " " + ($config.system.hardware || ""); 
		}
		$scope.$apply(); 
	}); 
	$scope.guiModes = [
		{id: "basic", label: gettext("Basic Mode")},
		{id: "expert", label: gettext("Expert Mode")},
		{id: "logout", label: gettext("Log out")}
	]; 
	Object.keys($scope.guiModes).map(function(k){
		var m = $scope.guiModes[k]; 
		if(m.id == $config.mode) $scope.selectedMode = m;
	});  
	$scope.onChangeMode = function(item){
		var selected = item.id; 
		if(selected == "logout") {
			$session.logout().always(function(){
				$window.location.href="/"; 
			}); 
		} else {
			$localStorage.setItem("mode", selected); 
			$config.mode = selected; 
			$state.reload(); 
		}
	};
}); 
