$juci.module("router")
.controller("SettingsPasswordCtrl", function($scope){
	$scope.showPassword = 0; 
	$scope.onChangePasswordClick = function(){
		$rpc.luci2.system.password_set({user: "martin", password: "asdf"}).done(function(data){
			console.log(JSON.stringify(data)); 
		}); 
	}
}); 
