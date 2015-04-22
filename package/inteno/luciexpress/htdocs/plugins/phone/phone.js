$juci.module("phone")
.state("phone", {
	url: "/phone", 
	views: {
		"content": {
			templateUrl: "pages/phone.call_log.html", 
		}
	},
	onEnter: function($state){
		$juci.redirect("phone.call_log"); 
	},
}); 
