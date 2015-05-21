//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

JUCI.app
.controller("StatusEventsPageCtrl", function($scope, $rpc){
	$scope.selectedShowType = "all"; 
	$scope.selectedLogTypes = []; 
	$rpc.luci2.system.syslog().done(function(result){
		if(result && result.log){
			$scope.logs = result.log.split("\n").map(function(line){
				var fields = line.match(/(\w* \w* \w* \d{2}:\d{2}:\d{2} \d{4}) ([^\s]*) ([^\s:]*): (.*)/); 
				if(fields){
					// remove first one because it is the whole line
					fields.shift(); 
					fields[0] = new Date(fields[0]); 
				}
				return fields; 
			})
			.filter(function(x){ return x != null && x[2] != "kernel"; }) // filter out all invalid matches 
			.reverse() // sort by date in descending order
			.map(function(x){ // convert date back to string and shorten it's format
				x[0] = x[0].toLocaleFormat("%d-%b-%Y %H:%M:%S"); 
				return x; 
			}); 
			$scope.$apply(); 
		}
	}); 
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
