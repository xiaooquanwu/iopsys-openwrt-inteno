$juci.module("core")
.directive("uciWirelessInterfaceMacfilterEdit", function($compile){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.wireless.interface.macfilter.edit.html", 
		scope: {
			interface: "=ngModel"
		}, 
		controller: "uciWirelessInterfaceMacfilterEditController", 
		replace: true, 
		require: "^ngModel"
	 };  
}).controller("uciWirelessInterfaceMacfilterEditController", function($scope, $rpc, $uci, $hosts){
	
	$scope.maclist = []; 
	$scope.filterEnabled = 0; 
	
	// updates scratch model for the view
	function updateMaclist(i){
		var maclist = []; 
		if(i && i.maclist) {
			async.eachSeries(i.maclist, function(mac, next){
				$hosts.select({ macaddr: mac }).done(function(host){
					maclist.push(host); 
					next(); 
				}).fail(function(){
					$hosts.insert({ hostname: "", macaddr: mac }).done(function(host){
						maclist.push(host);
					}).always(function(){ next(); });  
				}); 
			}, function(){
				$scope.maclist = maclist; 
			}); 
		} else {
			$scope.maclist = []; 
		}
	}
	
	// watch for model change
	$scope.$watch("interface", function(i){
		$scope.filterEnabled = i.macfilter; 
		updateMaclist(i);
	}, true); 
	
	// watch maclist for changes by the user
	$scope.$watch("maclist", function(list){
		// rebuild the maclist? 
		if(list && $scope.interface){
			var interface = $scope.interface; 
			interface.maclist = []; 
			list.map(function(x){
				// save the hostname 
				var macaddr = x.macaddr||""; 
				interface.maclist.push(macaddr); 
			}); 
		}
	}); 
	
	$scope.$watch("filterEnabled", function(value){ 
		$scope.interface.macfilter = value; 
	}); 
	
	$rpc.router.clients().done(function(clients){
		$scope.client_list = Object.keys(clients).map(function(x){ 
			return {
				checked: false, 
				client: clients[x]
			}
		});
		$scope.$apply(); 
	}); 
	
	$scope.onDeleteHost = function(host){
		$scope.maclist = ($scope.maclist||[]).filter(function(x) { return x.macaddr != host.macaddr; }); 
	}
	
	$scope.onAddClients = function(){
		// reset all checkboxes 
		if($scope.client_list){
			$scope.client_list.map(function(x){ x.checked = false; }); 
		}
		$scope.showModal = 1; 
	}
	
	$scope.onFilterEnable = function(){
		$scope.filterEnabled = !$scope.filterEnabled; 
		//console.log("Filter: "+$scope.filterEnabled); 
	}
	
	$scope.onAddNewClient = function(){
		var interface = $scope.interface; 
		if(interface){
			if(!interface.maclist) interface.maclist = []; 
			$scope.maclist.push({ hostname: "", macaddr: "" }); 
		}
	}
	
	$scope.onAcceptModal = function(){
		if($scope.client_list && $scope.maclist) {
			$scope.client_list.map(function(x){
				if(x.checked) {
					if($scope.maclist.filter(function(a) { return a.macaddr == x.client.macaddr; }).length == 0){
						$scope.maclist.push({ hostname: x.client.hostname, macaddr: x.client.macaddr }); 
					} else {
						console.log("MAC address "+x.client.macaddr+" is already in the list!"); 
					}
				}
			}); 
		}
		$scope.showModal = 0; 
	}
	
	$scope.onDismissModal = function(){
		$scope.showModal = 0; 
	}
}); 
