JUCI.app
.controller("StatusNATPageCtrl", function($scope, $rpc){
	$rpc.luci2.network.conntrack_table().done(function(table){
		if(table && table.entries){
			$scope.connections = table.entries.sort(function(a, b){ return (a.src+a.dest) < (b.src+b.dest); }); 
			$scope.$apply(); 
		}
	}); 
}); 
