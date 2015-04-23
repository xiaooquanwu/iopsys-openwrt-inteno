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
.factory('$tr', function(gettextCatalog, $localStorage) {
	return function(str){
		return gettextCatalog.getString(str); 
	}
});

angular.module("luci")
.factory('$languages', function($config, gettextCatalog, $localStorage, $window) {
	gettextCatalog.currentLanguage = $localStorage.getItem("language") || "se"; 
	return {
		getLanguages: function(){
			return Object.keys($config.languages).filter(function(lang){
				return lang in gettextCatalog.strings; 
			}).map(function(lang){
				return {
					title: $config.languages[lang].title, 
					short_code: lang
				}
			}); 
		}, 
		setLanguage: function(short_code){
			gettextCatalog.currentLanguage = short_code; 
			$localStorage.setItem("language", short_code); 
		}
	}
});
