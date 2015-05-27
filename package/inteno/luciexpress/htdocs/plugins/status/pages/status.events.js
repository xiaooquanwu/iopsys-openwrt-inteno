//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>

JUCI.app
.controller("StatusEventsPageCtrl", function($scope, $rpc){
	$scope.selectedShowType = "all"; 
	$scope.selectedLogTypes = ["system", "network"]; 
	
	var groups = {
		"system": [], 
		"network": ["netifd", "brcmnetlink"], 
	}; 
	
	JUCI.interval.repeat("syslog", 1000, function(done){
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
				.filter(function(x){ 
					if(x == null) return false; 
					var visible = false; 
					var error = false, warning = false; 
					if(x[1].indexOf("error") >= 0) error = true; 
					if(x[1].indexOf("warn") >= 0) warning = true; 
					if($scope.selectedShowType === "errwarn" && (!error && !warning)) return false; 
					$scope.selectedLogTypes.map(function(t){
						if(groups[t] && groups[t].indexOf(x[2]) >= 0) visible = true; 
					}); 
					return x != null && x[2] != "kernel" && x[2] != "syslog" && visible; 
				}) // filter out all invalid matches 
				.reverse() // sort by date in descending order
				.map(function(x){ // convert date back to string and shorten it's format
					x[0] = x[0].toLocaleFormat("%d-%b-%Y %H:%M:%S"); 
					return x; 
				}); 
				$scope.$apply(); 
				done(); 
			}
		}); 
	}); 
	$scope.allLogTypes = [
		{ label: "System", value: "system" }, 
		//{ label: "WAN", value: "wan" }, 
		{ label: "Network", value: "network" }, 
		{ label: "Other", value: "other" }, 
		//{ label: "LAN", value: "lan" }, 
		//{ label: "Voice", value: "voice" }, 
		//{ label: "Data", value: "data" }, 
		//{ label: "IPTV", value: "iptv" }, 
		//{ label: "USB", value: "usb" }, 
		//{ label: "Firewall", value: "firewall" }
	]; 
	$scope.allEventTypes = [
		{ label: "Only Errors & Warnings", value: "errwarn" }, 
		{ label: "All Events", value: "all" }
	];
	
	function onChange(){
		console.log(JSON.stringify($scope.selectedLogTypes) + $scope.selectedShowType); 
	}
	$scope.onTypeChanged = function(value){
		$scope.selectedShowType = value; 
	}; 
	$scope.$watchCollection("selectedLogTypes", function(){ onChange(); }); 
}); 
