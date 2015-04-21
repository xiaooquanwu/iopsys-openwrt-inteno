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
