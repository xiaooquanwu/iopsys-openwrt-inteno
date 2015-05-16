require("../../../../tests/lib-juci"); 

var completed = {
	"call_log": 1, 
	"number_blocking": 1, 
	"ringing_schedule": 0, 
	"speed_dialing": 0, 
	"numbers": 0
}

describe("Phone plugin", function(){
	it("should be completed", function(){
		expect(completed.call_log).to.be(1); 
		expect(completed.number_blocking).to.be(1); 
		expect(completed.ringing_schedule).to.be(1); 
		expect(completed.speed_dialing).to.be(1); 
		expect(completed.numbers).to.be(1); 
	}); 
}); 
