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
	"ipaddr":					{ dvalue: '', type: String }, 
	"netmask":				{ dvalue: '', type: String }, 
	"type":						{ dvalue: '', type: String }, 
	"ip6assign":			{ dvalue: 60, type: Number }, 
	"bridge_instance": { dvalue: false, type: Boolean }, 
	"vendorid":				{ dvalue: '', type: String }, 
	"hostname":				{ dvalue: '', type: String }, 
	"ipv6":						{ dvalue: false, type: Boolean },
	"peerdns": 				{ dvalue: false, type: String }, 
	"dns": 						{ dvalue: [], type: Array }
}); 

UCI.$registerConfig("dhcp"); 
UCI.dhcp.$registerSectionType("dhcp", {
	"interface": 		{ dvalue: "", type: String }, 
	"start": 				{ dvalue: 0, type: Number }, 
	"limit": 				{ dvalue: 0, type: Number }, 
	"leasetime": 		{ dvalue: 60, type: Number },
	"ignore": 			{ dvalue: false, type: Boolean }
}); 

UCI.$registerConfig("ddns");
UCI.ddns.$registerSectionType("service", {
    "enabled":              { dvalue: 0, type: Number },
    "interface":            { dvalue: "", type: String },
    "use_syslog":           { dvalue: 0, type: Number },
    "service_name":         { dvalue: "", type: String },
    "domain":               { dvalue: "", type: String },
    "username":             { dvalue: "", type: String },
    "password":             { dvalue: "", type: String }
});
