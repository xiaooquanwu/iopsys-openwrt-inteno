$juci.module("wifi")
.controller("WifiSchedulePageCtrl", function($scope, $uci, gettext){
	$scope.statusItems = [
		{ label: gettext("Enabled"), value: 1 },
		{ label: gettext("Disabled"), value: 0 }
	]; 
	$scope.editedData = { days: [], time: "123 "}; 
	
	$uci.sync(["wireless"]).done(function(){
		console.log("Got status"); 
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
		if($scope.schedule[".new"] == true) 
			$scope.schedule.$delete(); 
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
		console.log("Editing: "+sched[".name"]); 
		$scope.schedule = sched; 
		$scope.showScheduleDialog = 1; 
	}
	$scope.onDeleteSchedule = function(sched){
		sched.$delete().always(function(){
			$scope.$apply(); 
		}); 
	}
}); 
