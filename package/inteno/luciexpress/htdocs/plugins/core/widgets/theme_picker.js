$juci.module("core")
.directive("juciThemePicker", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	
	return {
		templateUrl: plugin_root+"/widgets/theme_picker.html", 
		replace: true
	 };  
})
.controller("JuciThemePickerController", function($scope, $theme, $config){
	$scope.themes = $config.themes.map(function(x){
		var item = {
			id: x, 
			label: x
		}; 
		if(x == $theme.getCurrentTheme()) $scope.selectedTheme = item; 
		return item; 
	}); 
	$scope.onChangeTheme = function(){
		//alert($scope.selectedTheme.id); 
		/*$theme.changeTheme($scope.selectedTheme.id).done(function(){
			window.location.reload(); 
		}); */
	}
}); 
