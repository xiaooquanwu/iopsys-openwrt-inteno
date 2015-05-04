$juci.module("router")
.controller("InternetPortMappingPageCtrl", function($scope, $uci, ModalService, $rpc){
	
	function reload(){
		$uci.sync("firewall").done(function(firewall){
			$scope.redirects = $uci.firewall["@forwarding"]; 
			$scope.$apply(); 
		}); 
	} reload(); 
	
	$scope.showModal = 0; 
	$scope.onAcceptModal = function(){
		var rule = $scope.rule; 
		//Object.keys($scope.redirects).map(function(idx) { var x = $scope.redirects[idx]; if(x[".name"] == rule[".name"]) $scope.redirects[idx] = rule; }); 
		console.log(JSON.stringify(rule)); 
		if(!rule[".name"]){
			// set up the rule in uci
			rule[".type"] = "forwarding"; 
			$uci.firewall.create(rule).done(function(rule){
				$scope.rule_src = rule; 
				$scope.$apply(); 
			}); 
		} 
		Object.keys(rule).map(function(key){
			if($scope.rule_src && key in $scope.rule_src) $scope.rule_src[key].value = rule[key]; 
		}); 
		$scope.showModal = 0;  
	}
	$scope.onDismissModal = function(){
		$scope.showModal = 0;
	}
	$scope.onAddRule = function(){
		$scope.rule = {};
		//$scope.redirects.push($scope.rule);  
		$scope.showModal = 1; 
	}
	$scope.onEditRule = function(rule){
		$scope.rule_src = rule; 
		$scope.rule = {
			".name": rule[".name"], 
			dest_ip: rule.dest_ip.value, 
			proto: rule.proto.value,
			dest_port: rule.dest_port.value, 
			src_dport: rule.src_dport.value 
		}; 
		$scope.modalTitle = "Edit port mapping ("+(rule['.name'] || 'new')+")"; 
		$scope.showModal = 1; 
	}
	$scope.onDeleteRule = function(rule){
		rule.$delete().done(function(){
			$scope.$apply(); 
		}); 
		/*if(rule[".name"]){
			$uci.delete("firewall."+rule[".name"]).always(function(){
				removeFromList(); 
			}); 
		} else {
			retmoveFromList(); 
		}*/
	}
}); 
