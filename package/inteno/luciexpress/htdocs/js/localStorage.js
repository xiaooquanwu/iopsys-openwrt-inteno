//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

// service for managing session data
angular.module("luci")
.factory('$localStorage', function() {
	return {
		getItem: function(item){
			return localStorage.getItem(item); 
		}, 
		setItem: function(item, value){
			return localStorage.setItem(item, value); 
		}
	}; 
});
