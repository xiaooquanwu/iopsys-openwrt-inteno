$juci.module("wifi")
.controller("WifiSchedulePageCtrl", function($scope, $uci){
	$uci.sync(["wireless"]).done(function(){
		$scope.status = $uci.wireless.status; 
		$scope.schedules = $uci.wireless["@wifi-schedule"]; 
		$scope.$apply(); 
	}).fail(function(err){
		console.log("failed to sync config: "+err); 
	}); 
	
	$scope.onAcceptSchedule = function(schedule){
		$uci.save().done(function(){
			$scope.showScheduleDialog = 0; 
			$scope.$apply(); 
		}).fail(function(error){
			alert("Error saving config! "+error);
		}); 
	}
	
	$scope.onDismissSchedule = function(schedule){
		$scope.showScheduleDialog = 0; 
	}
	
	$scope.onAddSchedule = function(){
		$uci.wireless.create({".type": "wifi-schedule"}).done(function(item){
			$scope.schedule = item; 
			$scope.showScheduleDialog = 1; 
			$scope.$apply(); 
			console.log("Added new schedule!"); 
		}).fail(function(err){
			console.log("Failed to create schedule!"); 
		}); ; 
	}
	
	$scope.onEditSchedule = function(sched){
		$scope.schedule = sched; 
		$scope.showScheduleDialog = 1; 
	}
	$scope.onDeleteSchedule = function(sched){
		sched.$delete().always(function(){
			$scope.$apply(); 
		}); 
	}
}); 
