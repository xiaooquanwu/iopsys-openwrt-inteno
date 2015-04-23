/*
 * juci - javascript universal client interface
 *
 * Project Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 * 
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
 
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
