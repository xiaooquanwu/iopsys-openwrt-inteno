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
}).controller("uciWirelessInterfaceMacfilterEditController", function($scope, $rpc, $uci){
	
	$scope.maclist = []; 
	$scope.filterEnabled = 0; 
	
	// updates scratch model for the view
	function updateMaclist(i){
		$scope.maclist = []; 
		if(i && i.maclist) {
			var clients = $scope.clients || {}; 
			i.maclist.map(function(x){
				var parts = x.split("|"); 
				$scope.maclist.push({ hostname: (clients[x]||parts[1]||""), macaddr: parts[0] }); 
			}); 
		}
	}
	
	// watch for model change
	$scope.$watch("interface", function(i){
		updateMaclist(i); 
	}); 
	
	// watch for changes in list of connected clients
	$scope.$watch("clients", function(clients){
		updateMaclist($scope.interface); 
	}); 
	
	// watch maclist for changes by the user
	$scope.$watch("maclist", function(list){
		// rebuild the maclist? 
		if(list && $scope.interface){
			var interface = $scope.interface; 
			interface.maclist = []; 
			list.map(function(x){
				// save the hostname 
				x.macaddr = x.macaddr||""; 
				x.hostname = x.hostname||""; 
				interface.maclist.push(x.macaddr.replace("|", "")+"|"+x.hostname.replace("|", "")); 
			}); 
		}
	}, true); 
	
	$scope.$watch("filterEnabled", function(value){ 
		$scope.interface.macfilter = value; 
	}); 
	$scope.$watch("interface.macfilter", function(value){
		$scope.filterEnabled = (value && value == "1"); 
	}); 
	$rpc.router.clients().done(function(clients){
		$scope.clients = {}; 
		$scope.client_list = []; 
		Object.keys(clients).map(function(x){ 
			var cl = clients[x]; 
			$scope.clients[cl.macaddr] = cl.hostname; 
			$scope.client_list.push({
				checked: false, 
				client: cl
			}); 
		});
		console.log(JSON.stringify($scope.clients)); 
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
					if($scope.maclist.filter(function(x) { return x.macaddr == x.client.macaddr; }).length == 0){
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
