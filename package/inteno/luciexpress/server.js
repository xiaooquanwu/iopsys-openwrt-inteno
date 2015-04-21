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
	
	if(data.params[1] == "luci2.ui" && data.params[2] == "menu"){
		console.log("luci2.ui.menu"); 
		res.write(JSON.stringify({
			jsonrpc: "2.0", 
			result: [0, {
				menu: JSON.parse(fs.readFileSync("share/menu.d/overview.json"))
			}]
		})); 
	} if(data.params[1] == "session" && data.params[2] == "access"){
		console.log("luci2.ui.menu"); 
		res.write(JSON.stringify({
			jsonrpc: "2.0", 
			result: [0, {
				"access-group": [ "a", "b" ]
			}]
		})); 
	} else {
		res.write(JSON.stringify({
			jsonrpc: "2.0",
			result: {}
		})); 
	}
	res.end(); 
});

var server = app.listen(3000, function () {

  var host = server.address().address;
  var port = server.address().port;

  console.log('Example app listening at http://%s:%s', host, port);

});

