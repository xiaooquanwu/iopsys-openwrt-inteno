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
