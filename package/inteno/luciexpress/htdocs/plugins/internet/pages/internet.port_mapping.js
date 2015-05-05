$juci.module("internet")
.controller("InternetPortMappingPageCtrl", function($scope, $uci, ModalService, $rpc){
	
	function reload(){
		$uci.sync("firewall").done(function(){
			$scope.redirects = $uci.firewall["@redirect"]; 
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
			rule[".type"] = "redirect"; 
			$uci.firewall.create(rule).done(function(rule){
				$scope.rule_src = rule; 
			}); 
		} 
		$scope.showModal = 0;  
	};
	$scope.onDismissModal = function(){
		$scope.showModal = 0;
	};
	$scope.onAddRule = function(){
		$scope.rule = {};
		//$scope.redirects.push($scope.rule);  
		$scope.showModal = 1; 
	};
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
	};
	$scope.onDeleteRule = function(rule){
		function removeFromList(){
			if($scope.redirects) {
				$scope.redirects = $scope.redirects.filter(function(x){ return x !== rule; }); 
				$scope.$apply(); 
			}
		}
		console.log("Deleting rule: "+rule[".name"]); 
		if(rule[".name"]){
			$uci.delete("firewall."+rule[".name"]).always(function(){
				removeFromList(); 
			}); 
		} else {
			removeFromList();
		}
	};
	$scope.onCommit = function(){
		if(!$scope.redirects) return; 
		$uci.commit("firewall").always(function(){
			$scope.$apply(); 
			console.log("Saved firewall settings!"); 
		}); 
	};
	$scope.onCancel = function(){
		$uci.rollback().always(function(){
			reload(); 
		}); 
	}
}); 
