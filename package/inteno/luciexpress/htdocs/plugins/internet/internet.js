JUCI.app
.config(function($stateProvider) {
	var plugin_root = $juci.module("internet").plugin_root; 
	$stateProvider.state("internet", {
		url: "/internet", 
		onEnter: function(){
			$juci.redirect("internet-firewall"); 
		}
	}); 
});

UCI.$registerConfig("network"); 
UCI.network.$registerSectionType("interface", {
	"is_lan":					{ dvalue: false, type: Boolean }, 
	"ifname":					{ dvalue: '', type: String }, 
	"proto":					{ dvalue: 'dhcp', type: String }, 
	"ipaddr":					{ dvalue: '127.0.0.1', type: String }, 
	"netmask":				{ dvalue: '255.0.0.0', type: String }, 
	"type":						{ dvalue: '', type: String }, 
	"ip6assign":			{ dvalue: 60, type: Number }, 
	"bridge_instance": { dvalue: false, type: Boolean }, 
	"vendorid":				{ dvalue: '', type: String }, 
	"hostname":				{ dvalue: '', type: String }, 
	"ipv6":						{ dvalue: false, type: Boolean }
}); 
