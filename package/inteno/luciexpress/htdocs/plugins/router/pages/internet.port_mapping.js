$juci.module("router")
.controller("InternetPortMappingPageCtrl", function($scope, $uci, ModalService, $rpc){
	
	function reload(){
		$uci.show("firewall").done(function(firewall){
			$scope.redirects = Object.keys(firewall).filter(function(x) { return firewall[x][".type"] == "redirect"; }).map(function(x){ return firewall[x]; }); 
			
			$scope.$apply(); 
		}); 
		
		
	} reload(); 
	$scope.showModal = 0; 
	$scope.onAcceptModal = function(){
		var rule = $scope.rule; 
		Object.keys($scope.redirects).map(function(idx) { var x = $scope.redirects[idx]; if(x[".name"] == rule[".name"]) $scope.redirects[idx] = rule; }); 
		console.log(JSON.stringify(rule)); 
		if(!rule[".name"]){
			// set up the rule in uci
			$uci.add("firewall", "redirect", rule).done(function(x){
				console.log("Added new section: "+x); 
				if(x.section) rule[".name"] = x.section; 
			}); 
		} else {
			$uci.set("firewall."+rule[".name"], rule).always(function(firewall){
				
			});
		}
		$scope.showModal = 0;  
	}
	$scope.onDismissModal = function(){
		$scope.showModal = 0;
	}
	$scope.onAddRule = function(){
		$scope.rule = {};
		$scope.redirects.push($scope.rule);  
		$scope.showModal = 1; 
	}
	$scope.onEditRule = function(rule){
		$scope.rule = Object.create(rule); 
		$scope.modalTitle = "Edit port mapping ("+(rule['.name'] || 'new')+")"; 
		$scope.showModal = 1; 
	}
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
			retmoveFromList(); 
		}
	}
	$scope.onCommit = function(){
		if(!$scope.redirects) return; 
		$uci.commit("firewall").always(function(){
			$scope.$apply(); 
			console.log("Saved firewall settings!"); 
		}); 
	}
	$scope.onCancel = function(){
		$uci.rollback().always(function(){
			reload(); 
		}); 
	}
}); 
