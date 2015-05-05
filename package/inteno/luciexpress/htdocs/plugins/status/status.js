$juci.module("status")
.state("status", {
	url: "/status", 
	onEnter: function($state){
		$juci.redirect("status.status"); 
	},
}); 
