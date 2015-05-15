global.assert = require("assert"); 
global.JSON = require("JSON"); 
global.async = require("async"); 
global.$ = global.jQuery = require("jquery-deferred"); 
global.$.ajax = require("najax"); 
global.async = require("async"); 
global.expect = require("expect.js"); 
require("../htdocs/lib/js/jquery-jsonrpc"); 
require("../htdocs/js/rpc"); 
require("../htdocs/js/uci"); 
var $rpc = global.$rpc = global.UBUS; 
var $uci = global.$uci = global.UCI; 
Object.prototype.assign = require("object-assign"); 

var host = "localhost"; 
var username; 
var password; 

for(var i = 0; i < process.argv.length; i++){
	switch(process.argv[i]){
		case "--pass": password = process.argv[++i]; break; 
		case "--user": username = process.argv[++i]; break; 
		case "--host": host = process.argv[++i]; break; 
	}; 
}

if(!username || !password ){
	console.error("Please specify --user <rpcuser> and --pass <rpcpassword> arguments!"); 
	process.exit(); 
}

function init(){
	var deferred = $.Deferred(); 
	async.series([
		function(next){
			console.log("Trying to connect to RPC host '"+host+"'..."); 

			$rpc.$init({host: "http://"+host}).done(function(){
				//console.log("Initialized rpc: "+Object.keys($rpc)); 
				next(); 
			}).fail(function(){
				throw new Error("Could not connect to rpc host!"); 
			}); 
		}, 
		function(next){
			$rpc.$login({username: username, password: password}).done(function(){
				next(); 
			}).fail(function(){
				throw new Error("Could not login over RPC using specified username and password!"); 
			}); 
		}, 
		function(next){
			$uci.$init().done(function(){
				//console.log("Initialized uci: "+Object.keys($uci)); 
				next(); 
			}); 
		}
	], function(){
		deferred.resolve(); 
	}); 
	return deferred.promise(); 
}

before(function(done){
	init().done(function(){
		done(); 
	}); 
}); 

exports.$uci = $uci; 
exports.$init = init; 
exports.$rpc = $rpc; 
exports.app = {
	config: function(func){
		//func(); 
	}
}
