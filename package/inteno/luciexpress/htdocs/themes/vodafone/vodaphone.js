(function(){
	var overrides = {
		"luciFooterDirective": "/widgets/luci.footer.html", 
		"luciLayoutNakedDirective": "/widgets/luci.layout.naked.html", 
		"luciLayoutSingleColumnDirective": "/widgets/luci.layout.single_column.html", 
		"luciLayoutWithSidebarDirective": "/widgets/luci.layout.with_sidebar.html", 
		"luciNavDirective": "/widgets/luci.nav.html", 
		"luciNavbarDirective": "/widgets/luci.navbar.html", 
		"luciTopBarDirective": "/widgets/luci.top_bar.html"
	}; 
	Object.keys(overrides).map(function(k){
		$juci.decorator(k, function($delegate){
			var plugin_root = $juci.module("vodafone").plugin_root; 
			$delegate[1].templateUrl = plugin_root + overrides[k]; 
			return $delegate; 
		}); 
	}); 
})(); 
