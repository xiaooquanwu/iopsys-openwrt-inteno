$juci.module("wifi")
.state("wifi", {
	url: "/wifi", 
	views: {
		"content": {
			templateUrl: "pages/wifi.html", 
		}
	}, 
	onEnter: function($state){
		
		$juci.redirect("wifi-general"); 
	}
}); 
