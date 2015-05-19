$juci.module("settings")
.controller("SettingsPasswordCtrl", function($scope, $rpc, $tr, $session, gettext){
	$scope.showPassword = 0; 
	$scope.showModal = 0; 
	$scope.modal = {
		old_password: "", 
		password: "", 
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
		$scope.modal = {}; 
		$scope.showModal = 1; 
	}
	$scope.onAcceptModal = function(){
		if($scope.modal.password != $scope.modal.password2) alert($tr(gettext("Passwords do not match!"))); 
		else {
			// TODO: change to correct username
			$rpc.luci2.system.password_set({user: $rpc.$session.data.username, password: $scope.modal.password, curpass: $scope.modal.old_password}).done(function(data){
				$scope.showModal = 0; 
				$scope.$apply(); 
			}).fail(function(){
				
			}); 
		}
	}
	$scope.onDismissModal = function(){
		$scope.showModal = 0; 
	}
}); 

