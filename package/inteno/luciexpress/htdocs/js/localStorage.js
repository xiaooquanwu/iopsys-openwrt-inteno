//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

(function(){
	function supports_html5_storage() {
		try {
			return 'localStorage' in window && window['localStorage'] !== null;
		} catch (e) {
			return false;
		}
	}
	var fake_storage = {}; 
	function JUCILocalStorage(){
		this.getItem = function(item){ 
			if(supports_html5_storage()) return localStorage.getItem(item); 
			else return fake_storage[item]; 
		}; 
		this.setItem = function(item, value){
			if(supports_html5_storage()) return localStorage.setItem(item, value); 
			else fake_storage[item] = value; 
			return fake_storage[item]; 
		}
	}
	$juci.localStorage = new JUCILocalStorage(); 
	
	JUCI.app.factory('$localStorage', function() {
		return $juci.localStorage; 
	});
})(JUCI); 

