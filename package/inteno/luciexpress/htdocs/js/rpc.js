//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

// luci rpc module for communicating with the server
angular.module("luci")
.factory('$rpc', function($rootScope, $config, gettext){
	var rpc = window.rpc = {
		register_method: function(call){
			//console.log("registering: "+call); 
			var self = this; 
			function _find(path, obj){
				if(!obj.hasOwnProperty(path[0])){
					obj[path[0]] = {}; 
				}
				if(path.length == 1) {
					var namespace = call.split("."); 
					namespace.pop(); namespace = namespace.join("."); 
					(function(namespace, method){
						// create the rpc method
						obj[path[0]] = function(data){
							var func = (function(data){
								if(!data) data = { }; 
								var deferred = $.Deferred(); 
								$.jsonRPC.withOptions({
									namespace: "", 
									endPoint: (($config.rpc.host)?$config.rpc.host:"")+"/ubus"
								}, function(){	 
									var sid = "00000000000000000000000000000000"; 
									if($rootScope.sid) sid = $rootScope.sid; 
									//data.ubus_rpc_session = sid;  
									this.request('call', {
										params: [ sid, namespace, method, data],
										success: function(result){
											//alert("SID: "+sid + " :: "+ JSON.stringify(result)); 
											if(result && result.result) {
												// TODO: modify all rpc UCI services so that they ALWAYS return at least 
												// an empty json object. Otherwise we have no way to differentiate success 
												// from failure of a request. This has to be done on the host side. 
												if(result.result[0] != 0){ // || result.result[1] == undefined) {
													console.log("RPC succeeded, but returned error: "+JSON.stringify(result));
													deferred.reject((function(){
														switch(result.result[0]){
															case 6: return gettext("Access denied!"); 
															default: return gettext("RPC error #")+result.result[0]+": "+result.result[1]; 
														}
													})()); 
												} else {
													deferred.resolve(result.result[1]);
												}
											} else {
												deferred.reject(); 
											}
										}, 
										error: function(result){
											console.error("RPC error ("+namespace+"."+method+"): "+JSON.stringify(result));
											if(result && result.error){
												deferred.reject(result.error);  
												$rootScope.$broadcast("error", result.error.message); 
											}
										}
									})
								});
								return deferred.promise(); 
							}); 
							return func(data); 
						}
					})(namespace, path[0]); 
				} else {
					var child = path[0]; 
					path.shift(); 
					_find(path, obj[child]); 
				}
			}
			_find(call.split("."), self); 
		}
	}; 
	// setup default rpcs
	if($config.rpc && $config.rpc.exposed_calls){
		$config.rpc.exposed_calls.forEach(function(call){
			rpc.register_method(call); 
		}); 
	}
	return rpc; 
}); 
