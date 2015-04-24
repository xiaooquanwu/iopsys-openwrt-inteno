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
 
// uci module for interacting with uci tables
angular.module("luci")
.factory('$uci', function($rpc){
	function initReq(path){
		var parts = path.split("."); 
		var req = {}; 
		if(parts.length == 0) { deferred.reject(); return; }
		req.config = parts[0]; 
		if(parts.length > 1) req.section = parts[1]; 
		if(parts.length > 2) req.option = parts[2]; 
		return req; 
	}
	return {
		show: function(path){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			$rpc.uci.state(req).done(function(state){
				if(state && state.values) deferred.resolve(state.values); 
				else if(state && state.value) deferred.resolve(state.value); 
				else deferred.reject(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}, 
		set: function(path, values){
			var deferred = $.Deferred(); 
			var req = initReq(path); 
			req.values = values; 
			$rpc.uci.set(req).done(function(state){
				$rpc.uci.commit(req).done(function(){
					deferred.resolve(); 
				}).fail(function(){
					deferred.reject(); 
				}); 
			}).fail(function(){
				deferred.reject(); 
			}); 
			return deferred.promise(); 
		}
	}; 
}); 
