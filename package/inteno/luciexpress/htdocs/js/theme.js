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
 
// luci rpc module for communicating with the server
angular.module("luci")
.factory('$theme', function($rootScope, $config, $localStorage, $http){
	var calls = $config.rpc.exposed_calls; 
	return {
		currentTheme: null, 
		themes: {}, // TODO: also load themes inside this service
		loadTheme: function(theme_id){
			console.log("Loading theme "+theme_id); 
			var deferred = $.Deferred(); 
			
			var themes = this.themes; 
			if(!(theme_id in themes)) {
				var theme_root = "themes/"+theme_id; 
				$http.get(theme_root+"/theme.json").success(function(data){
					if(!data) return; 
					
					// create new module
					$juci.module(theme_id, theme_root, data); 
					
					themes[theme_id] = data; 
					if(data.scripts){
						async.eachSeries(data.scripts, function(script, next){
							console.log("Loading "+theme_root + "/"+script); 
							require([theme_root + "/"+script], function(module){
								next(); 
							}); 
						}, function(){
							deferred.resolve(data); 
						}); 
					} else {
						deferred.resolve(data); 
					}
				}).error(function(){
					console.log("Could not retreive theme config for theme: "+theme_id); 
				}); 
			} else {
				deferred.resolve(themes[theme_id]); 
			}
			
			return deferred.promise(); 
		},
		changeTheme: function(theme_id){
			var deferred = $.Deferred(); 
			this.loadTheme(theme_id).done(function(theme){
				$config.theme = theme_id; 
				$localStorage.setItem("theme", theme_id); 
				var theme_root = "themes/"+theme_id; 
				//$rootScope.theme_index = th+"/index.html"; 
				$("head link[data-theme-css]").remove(); 
				if(theme.styles){
					theme.styles.map(function(x){
						var style = $('<link href="'+theme_root+'/' + x + '" rel="stylesheet" data-theme-css/>');
						style.appendTo('head'); 
					}); 
				}
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
				// error
			}); 
			return deferred.promise(); 
		}, 
		getAvailableThemes: function(){
			return this.themes; 
		}
	}; 
	/*
	$(function(){
		var themes = $config.themes; 
		$config.theme = localStorage.getItem("theme") || "default"; 
		var bootstrap = $('<link href="'+themes[$config.theme]+'/css/bootstrap.min.css" rel="stylesheet" />');
		var theme = $('<link href="'+themes[$config.theme]+'/css/theme.css" rel="stylesheet" />');
		bootstrap.appendTo('head');
		theme.appendTo('head'); 
		$('.theme-link').click(function(){
			var themename = $(this).attr('data-theme');
			var themeurl = themes[themename];
			$config.theme = themename;
			localStorage.setItem("theme", themename);
			bootstrap.attr('href',themeurl+"/css/bootstrap.min.css");
			theme.attr('href',themeurl+"/css/theme.css");
			window.location.reload(); 
		});
	});*/
}); 
