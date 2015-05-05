$juci.module("router")
.controller("SettingsPasswordCtrl", function($scope){
	$scope.showPassword = 0; 
	$scope.showModal = 0; 
	$scope.modal = {
		old_password: "", 
		password: "test", 
		password2: ""
	}; 
	$scope.passwordStrength = 1; 
	
	
	function measureStrength(p) {
		var strongRegex = new RegExp("^(?=.{8,})(?=.*[A-Z])(?=.*[a-z])(?=.*[0-9])(?=.*\\W).*$", "g");
		var mediumRegex = new RegExp("^(?=.{7,})(((?=.*[A-Z])(?=.*[a-z]))|((?=.*[A-Z])(?=.*[0-9]))|((?=.*[a-z])(?=.*[0-9]))).*$", "g");
		var enoughRegex = new RegExp("(?=.{4,}).*", "g");
		
		if(strongRegex.test(p)) return 3; 
		if(mediumRegex.test(p)) return 2; 
		if(enoughRegex.test(p)) return 1; 
		return 0; 
	}
	
	$scope.$watch("modal", function(){
		$scope.passwordStrength = measureStrength($scope.modal.password); 
	}, true); 
	$scope.onChangePasswordClick = function(){
		$scope.showModal = 1; 
		/*$rpc.luci2.system.password_set({user: "martin", password: "asdf"}).done(function(data){
			$scope.showModal = 1; 
		}); */
	}
}); 

