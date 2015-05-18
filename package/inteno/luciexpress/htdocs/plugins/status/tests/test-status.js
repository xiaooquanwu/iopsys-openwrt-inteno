require("../../../../tests/lib-juci"); 

var completed = {
	"about": 1, 
	"diagnostics": 0, 
	"events": 0, 
	"nat": 0, 
	"restart": 1, 
	"status.dsl": 0, 
	"status.status": 1, 
	"status.tv": 0, 
	"status.voice": 0
}

describe("Status", function(){
	it("should be completed", function(){
		expect(Object.keys(completed).filter(function(x){ return completed[x] == 0; })).to.be.empty(); 
	}); 
	it("should have router.info rpc call", function(){
		expect($rpc.router).to.be.ok(); 
		expect($rpc.router.info).to.be.ok(); 
	}); 
}); 
