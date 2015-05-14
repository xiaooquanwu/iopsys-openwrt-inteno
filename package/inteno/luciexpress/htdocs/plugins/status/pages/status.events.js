$juci.module("status")
.controller("StatusEventsPageCtrl", function($scope){
	$scope.selectedShowType = "all"; 
	$scope.selectedLogTypes = []; 
	
	$scope.allLogTypes = [
		{ label: "System", value: "system" }, 
		{ label: "WAN", value: "wan" }, 
		{ label: "LAN", value: "lan" }, 
		{ label: "Voice", value: "voice" }, 
		{ label: "Data", value: "data" }, 
		{ label: "IPTV", value: "iptv" }, 
		{ label: "USB", value: "usb" }, 
		{ label: "Firewall", value: "firewall" }
	]; 
	$scope.allEventTypes = [
		{ label: "Only Errors & Warnings", value: "errwarn" }, 
		{ label: "All Events", value: "all" }
	]; 
}); 
