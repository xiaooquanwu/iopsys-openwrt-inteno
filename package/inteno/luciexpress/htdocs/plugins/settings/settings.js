$juci.module("settings")
.state("settings", {
	url: "/settings", 
	onEnter: function($state){
		$juci.redirect("settings-password"); 
	},
});
