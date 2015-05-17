require("../../../../tests/lib-juci"); 

var completed = {
	"dns": 0, 
	"exposed_host": 1, 
	"firewall": 1, 
	"port_mapping": 1
}

describe("Internet plugin", function(){
	it("should be completed", function(){
		expect(Object.keys(completed).filter(function(x){ return completed[x] == 0; })).to.be.empty(); 
	}); 
}); 

describe("UCI.firewall", function(){
	before(function(done){
		$uci.sync("firewall").done(function(){
			done(); 
		}); 
	}); 
	it("should have dmz section of type include", function(){
		expect($uci.firewall.dmz).to.be.ok(); 
		expect($uci.firewall.dmz[".type"]).to.be("include"); 
	}); 
	it("should have dmzhost section of type dmzhost", function(){
		expect($uci.firewall).to.have.property("dmzhost"); 
		expect($uci.firewall.dmzhost[".type"]).to.be("dmzhost"); 
	}); 
}); 
