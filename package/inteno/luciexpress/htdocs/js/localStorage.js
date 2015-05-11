//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

(function(){
	function JUCILocalStorage(){
		this.getItem = function(item){
			return localStorage.getItem(item); 
		}; 
		this.setItem = function(item, value){
			return localStorage.setItem(item, value); 
		}
	}
	$juci.localStorage = new JUCILocalStorage(); 
	
	angular.module("luci")
	.factory('$localStorage', function() {
		return $juci.localStorage; 
	});
})(JUCI); 

