require("../../../../tests/lib-juci"); 

var completed = {
	"dns": 0, 
	"exposed_host": 0, 
	"firewall": 1, 
	"port_mapping": 1
}

describe("Internet plugin", function(){
	it("should be completed", function(){
		expect(Object.keys(completed).filter(function(x){ return completed[x] == 0; })).to.be.empty(); 
	}); 
}); 
