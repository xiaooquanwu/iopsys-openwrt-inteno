JUCI.app
.controller("StatusNATPageCtrl", function($scope, $rpc){
	$rpc.luci2.network.conntrack_table().done(function(table){
		if(table && table.entries){
			$scope.connections = table.entries.sort(function(a, b){ return (a.src+a.dest) < (b.src+b.dest); }).map(function(x){
				switch(x.protocol){
					case 6: x.protocol = "TCP"; break; 
					case 2: x.protocol = "UNIX"; break; 
					case 17: x.protocol = "UDP"; break; 
				}
				return x; 
			}); 
			$scope.$apply(); 
		}
	}); 
}); 
