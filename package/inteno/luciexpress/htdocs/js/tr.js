//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

// service for managing session data
angular.module("luci")
.factory('$tr', function(gettextCatalog) {
	return function(str){
		return gettextCatalog.getString(str); 
	}
});

angular.module("luci")
.factory('$languages', function($config, gettextCatalog, $localStorage) {
	gettextCatalog.currentLanguage = $localStorage.getItem("language") || "se"; 
	return {
		getLanguages: function(){
			return $config.languages.filter(function(lang){
				return lang in gettextCatalog.strings; 
			}).map(function(lang){
				return {
					title: "language."+lang, 
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
