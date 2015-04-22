$juci.module("core")
	.directive("luciLogin", function(){
		var plugin_root = $juci.module("core").plugin_root; 
		return {
			// accepted parameters for this tag
			scope: {
			}, 
			templateUrl: plugin_root+"/widgets/luci.login.html", 
			replace: true, 
			controller: "LoginControl",
			controllerAs: "ctrl"
		}; 
	})
	.controller("LoginControl", function($scope, $session, $state, $window, $location){
		$scope.form = { "username": "", "password": "", "remember": 0 }; 
		$scope.loggedIn = $session.isLoggedIn(); 
		$scope.doLogin = function(){
			$session.login({
				"username": $scope.form.username, 
				"password": $scope.form.password, 
				"remember": $scope.form.remember
			}).done(function success(res){
				//$state.go("home", {}, {reload: true});
				$window.location.href="/"; 
			}).fail(function fail(res){
				
			}); 
		}
		$scope.doLogout = function(){
			$session.logout().done(function(){
				console.log("Logged out!"); 
				//$state.go("home", {}, {reload: true});
				$window.location.href="/"; 
			}).fail(function(){
				alert("Error logging out!");
			});  
		}
	}); 
