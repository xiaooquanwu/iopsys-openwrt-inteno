//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

// service for managing session data
(function(scope){
	var $juci = scope.JUCI;  
	var $rpc = scope.UBUS; 
	
	function JUCISession(){
		var saved_sid = $juci.localStorage.getItem("sid");
		var default_sid = "00000000000000000000000000000000";  
		if(saved_sid){
			$rpc.$sid(saved_sid); 
		} 
		
		this.sid = (saved_sid)?saved_sid:default_sid; 
		this.data = {}; 
		this.isLoggedIn = function(){
			return this._loggedIn; 
		}, 
		this.$init = function() {
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
					self.data = result; 
					console.log("Session: Loggedin!"); 
					self._loggedIn = true; 
					deferred.resolve(result); 
				}  
			}).fail(function err(result){
				self.sid = default_sid; 
				$juci.localStorage.setItem("sid", self.sid); 
				$rpc.$sid(self.sid); 
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}; 
		this.login = function(obj){
			var self = this; 
			var deferred  = $.Deferred(); 
			$rpc.session.login({
				"username": obj.username, 
				"password": obj.password
			}).done(function(result){
				self.sid = result.ubus_rpc_session;
				$rpc.$sid(self.sid); 
				self.data = result; 
				self._loggedIn = true; 
				$juci.localStorage.setItem("sid", self.sid); 
				//if(result && result.acls && result.acls.ubus) setupUbusRPC(result.acls.ubus); 
				deferred.resolve(self.sid); 
			}).fail(function(result){
				deferred.reject(result); 
			}); 
			return deferred.promise(); 
		}; 
		this.logout = function(){
			var deferred = $.Deferred(); 
			var self = this; 
			$rpc.session.destroy().done(function(){
				self.data = {}; 
				self._loggedIn = false; 
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
	}
	
	$juci.session = new JUCISession(); 
	
	JUCI.app.factory('$session', function($rpc, $rootScope, $localStorage) {
		return $juci.session; 
	});
})(typeof exports === 'undefined'? this : global); 
