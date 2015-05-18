require("../../../../tests/lib-juci"); 

var completed = {
	"configuration": 1, 
	"energy": 1, 
	"network": 0, 
	"password": 0, 
	"upgrade": 0
}

describe("Settings", function(){
	it("should be completed", function(){
		expect(Object.keys(completed).filter(function(x){ return completed[x] == 0; })).to.be.empty(); 
	}); 
}); 
