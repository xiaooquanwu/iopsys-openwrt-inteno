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
		return {
			id: x, 
			label: x
		}; 
	}); 
	$scope.onChangeTheme = function(item){
		$theme.changeTheme(item.id).done(function(){
			window.location.reload(); 
		}); 
	}
}); 
