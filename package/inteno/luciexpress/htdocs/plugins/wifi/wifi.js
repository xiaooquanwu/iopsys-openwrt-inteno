$juci.module("core")
.state("wifi", {
	url: "/wifi", 
	views: {
		"content": {
			templateUrl: "pages/default.html", 
		}
	}, 
	onEnter: function($state){
		
		$juci.redirect("wifi.general"); 
	}
}); 
