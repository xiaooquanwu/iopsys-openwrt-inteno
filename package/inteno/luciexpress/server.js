var express = require('express');
var app = express();
var JSON = require("JSON"); 
var fs = require("fs"); 

var bodyParser = require('body-parser')

/* 
Cross origin requests
To allow cross origin requests from your local setup to your router, 
add the following lines into target* -> uhttpd -> client -> uh_http_header() 

ustream_printf(cl->us, "Access-Control-Allow-Origin: *\r\n"); 
ustream_printf(cl->us, "Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept\r\n");  
*/

app.use( bodyParser.json() );       // to support JSON-encoded bodies
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
  extended: true
})); 

app.use(express.static(__dirname + '/htdocs'));

var rpc_calls = {
	"luci2.ui.menu": function(params, next){
		var menu = {}; 
		[
			"share/menu.d/overview.json", 
			"share/menu.d/settings.json",
			"share/menu.d/phone.json",
			"share/menu.d/internet.json",
			"share/menu.d/status.json",
			"share/menu.d/status.vodaphone.json",
			"share/menu.d/wifi.json"
		].map(function(file){
			var obj = JSON.parse(fs.readFileSync(file)); 
			Object.keys(obj).map(function(k){
				menu[k] = obj[k]; 
			});  
		}); 
		next({
			menu: menu
		}); 
	}, 
	"session.access": function(params, next){
		next({
			"access-group": [ "a", "b" ]
		}); 
	}
}; 

// RPC end point
app.post('/ubus', function(req, res) {
  res.header('Content-Type', 'application/json');
  var data = req.body, err = null, rpcMethod;
	
	console.log(JSON.stringify(data)); 
	
  if (!err && data.jsonrpc !== '2.0') {
    onError({
      code: -32600,
      message: 'Bad Request. JSON RPC version is invalid or missing',
      data: null
    }, 400);
    return;
  }
	
	//console.log("Call: "+data.method+" "+JSON.stringify(data.params)); 
	var name = data.params[1]+"."+data.params[2]; 
	if(name in rpc_calls){
		rpc_calls[name](null, function(resp){
			res.write(JSON.stringify({
				jsonrpc: "2.0", 
				result: [0, resp]
			}));
			
			res.end(); 
		}); 
	} else {
		console.log("Unknown RPC call "+name); 
		res.end(); 
	}
});

var server = app.listen(3000, function () {

  var host = server.address().address;
  var port = server.address().port;

  console.log('Example app listening at http://%s:%s', host, port);

});

