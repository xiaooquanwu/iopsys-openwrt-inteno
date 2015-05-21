(function(){
	var _timeouts = {}; 
	JUCI.interval = {
		once: function(t, fn){
			var i = setTimeout(function _onTimeout(){
				fn(function next(ret, err){
					clearTimeout(i); 
					delete _timeouts[name]; 
				}); 
			}, t); 
			_timeouts[name] = i; 
		}, 
		repeat: function(name, t, fn){
			function _onTimeout(){
				fn(function next(ret, err){
					if(ret){
						clearTimeout(i); 
						delete _timeouts[name]; 
					} else {
						// restart the timeout if it did not exit
						setTimeout(_onTimeout, t); 
					}
				}); 
			}
			var i = setTimeout(_onTimeout, t); _onTimeout(); 
			_timeouts[name] = i; 
		}, 
		$clearAll: function(){
			Object.keys(_timeouts).map(function(t){ 
				clearTimeout(t); 
			});
		} 
	}; 
})(); 
