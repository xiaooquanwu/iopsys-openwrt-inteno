//! Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 
$juci.module("phone")
.controller("PhoneCallLogPageCtrl", function($scope, $uci, gettext, $tr){ 
	$scope.phoneFilter = ""; 
	$scope.phoneFilterSelected = {}; 
	$scope.phoneList = []; 
	$scope.call_log = []; 
	$rpc.asterisk.call_log.list().done(function(res){
		var unique_phones = {}; 
		$scope.call_log = res.call_log.map(function(log){
			var date = new Date(log.time.replace(/CEST/g, "")); 
			var now = new Date(); 
			if(now.getDate() == date.getDate() && now.getMonth() == date.getMonth() && date.getFullYear() == now.getFullYear())
				log.date = $tr(gettext("Today")); 
			else 
				log.date = date.getDate() + ":"+date.getMonth()+":"+date.getFullYear(); 
			log.time = date.getHours()+":"+date.getMinutes(); 
			unique_phones[log.from] = true; 
			return log; 
		}); 
		$scope.phoneList = Object.keys(unique_phones).map(function(x){ return { from: x }; }); 
		$scope.phoneFilter = ""; 
		$scope.$apply(); 
	}); 
	$scope.onChangeFilter = function(item, model){
		$scope.phoneFilter = model.from; 
	}
}); 
