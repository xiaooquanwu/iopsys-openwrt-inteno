//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

(function($juci){
	function JUCIConfig(){
		this.system = {}; 
	}
	JUCIConfig.prototype.$init = function(){
		var deferred = $.Deferred(); 
		var self = this; 
		console.log("Init CONFIG"); 
		UCI.sync("juci").done(function(){
			if(UCI.juci && UCI.juci.settings){
				console.log("Using settings from config/juci on router"); 
				Object.keys(UCI.juci.settings).map(function(k){
					var i = UCI.juci.settings[k]; 
					if(i instanceof Object && "value" in i) self[k] = i.value; 
				}); 
				deferred.resolve(); 
			} else {
				loadJSON(); 
			}
		}).fail(function(){
			loadJSON(); 
		}); 
		function loadJSON(){
			console.log("Using settings from config.json on router"); 
			$.get("/config.json", {
				format: "json"
			}).done(function(data){
				if(!data || data == undefined) throw new Error("Could not get config.json!"); 
				Object.keys(data).map(function(k) { self[k] = data[k]; }); 
				deferred.resolve(); 
			}).fail(function(){
				deferred.reject(); 
			}); 
		}
		
		return deferred.promise(); 
	}
	$juci.config = new JUCIConfig(); 
	
	JUCI.app.factory('$config', function(){
		return $juci.config; 
	}); 
})(JUCI); 
