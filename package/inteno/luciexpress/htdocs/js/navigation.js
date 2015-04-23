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
 

// provides a service for managing all pages
// pages register with this service, and menus can query it to get the navigation tree
angular.module("luci")
.provider('$navigation', function navigationProvider($stateProvider){
	var data = {
		children: {},
		children_list: []
	}; 
	var self = this; 
	this.tree = function(path){
		if(!path)
			return data; 
		return this.findLeaf(path); 
	}
	this.findLeaf = function(path){
		var parts = path.split("."); 
		var obj = data; 
		// find the right leaf node
		while(parts.length){
			if(obj.children.hasOwnProperty(parts[0])){
				obj = obj.children[parts.shift()]; 
			} else {
				return null; 
			}
		} 
		return obj; 
	}
	this.insertLeaf = function(path, item){
		var parts = item.path.split("."); 
		var obj = data; 
		// find the right leaf node
		while(parts.length > 1){
			if(obj.children.hasOwnProperty(parts[0])){
				obj = obj.children[parts.shift()]; 
			} else {
				obj.children[parts[0]] = {
					children: {},
					children_list: []
				};
				obj = obj.children[parts.shift()]; 
			}
		} 
		if(!item.children) {
			item.children = {}; 
			item.children_list = []; 
		}
		if(!obj.children.hasOwnProperty(parts[0])){
			obj.children[parts[0]] = item; 
			obj.children_list.push(item); 
		} else {
			var o = obj.children[parts[0]]; 
			var children = o.children; 
			Object.assign(o, item);
			Object.assign(o.children, children); 
			item = o; 
		}
		obj.children_list = Object.keys(obj.children).map(key => obj.children[key]);
		obj.children_list.sort(function(a, b){
			return a.index - b.index; 
		}); 
		return item; 
	} 
	this.register = function(item){
		if(!item.path) return; 
		item = this.insertLeaf(item.path, item); 
		
		
		// now also register with the routing service 
		//if(item.page){
		/*	(function(item){
				var lazyPromise = null; 
				$stateProvider.state(item.path.replace(".", "_"), {
					url: "/"+item.path, 
					views: {
						"content": {
							templateUrl: item.page || "/pages/default.html"
						}
					},
					onEnter: function($window){
						// TODO: all these redirects seem to load page multiple times. 
						if(item.redirect) $window.location.href = "#!"+item.redirect; 
					},
					luci_config: item
				}); 
			})(item); */
		//}
		//alert(JSON.stringify(data)); 
		return data; 
	}; 
	this.$get = function() {
		return self; 
	}
}); 
