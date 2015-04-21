// service for managing session data
angular.module("luci")
.factory('$session', function($rpc, $rootScope) {
	var saved_sid = localStorage.getItem("sid");
	var default_sid = "00000000000000000000000000000000";  
	if(saved_sid){
		$rootScope.sid = saved_sid; 
	} 
	return {
		sid: (saved_sid)?saved_sid:default_sid, 
		data: {}, 
		isLoggedIn: function(){
			return Object.keys(this.data).length != 0; 
		}, 
		init: function() {
			var self = this; 
			var deferred = $.Deferred(); 
			console.log("Checking session key with server: "+saved_sid); 
			$rpc.session.access({
				"keys": ""
			}).done(function(result){
				if(result["access-group"] && result["access-group"].unauthenticated && Object.keys(result["access-group"]).length == 1) {
					console.log("Session: Not authenticated!"); 
					deferred.reject(); 
				} else {
					console.log("Session: Loggedin!"); 
					self.data = result; 
					deferred.resolve(result); 
				}  
			}).fail(function err(result){
				$rootScope.sid = self.sid = default_sid; 
				localStorage.setItem("sid", self.sid); 
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}, 
		login: function(obj){
			var self = this; 
			var deferred  = $.Deferred(); 
			$rpc.session.login({
				"username": obj.username, 
				"password": obj.password
			}).done(function(result){
				$rootScope.sid = self.sid = result.ubus_rpc_session;
				self.data = result; 
				localStorage.setItem("sid", self.sid); 
				deferred.resolve(self.sid); 
			}).fail(function(result){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}, 
		logout: function(){
			var deferred = $.Deferred(); 
			var self = this; 
			$rpc.session.destroy().done(function(){
				self.data = {}; 
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
	};
});
