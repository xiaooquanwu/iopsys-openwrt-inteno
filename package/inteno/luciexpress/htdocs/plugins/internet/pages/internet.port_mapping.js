$juci.app
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
		$uci.firewall.create({".type": "redirect"}).done(function(section){
			$scope.rule = section; 
			$scope.rule[".new"] = true; 
			$scope.rule[".edit"] = true; 
			$scope.showModal = 1;
			$scope.$apply(); 
		}); 
	};
	
	$scope.onEditRule = function(rule){
		$scope.rule = rule; 
		rule[".edit"] = true; 
	};
	
	$scope.onDeleteRule = function(rule){
		rule.$delete().done(function(){
			$scope.$apply(); 
		}); 
	};
	
	$scope.onAcceptEdit = function(rule){
		$uci.save().done(function(){
			rule[".edit"] = false; 
			$scope.$apply(); 
		}); 
	};
	
	$scope.onCancelEdit = function(rule){
		rule[".edit"] = false; 
		if(rule[".new"]){
			rule.$delete().done(function(){
				$scope.$apply(); 
			}); 
		} else {
			rule.$sync().done(function(){
				$scope.$apply(); 
			}); 
		}
	}
}); 
