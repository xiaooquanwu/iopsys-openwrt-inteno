$juci.module("wifi")
.directive("uciWirelessScheduleEdit", function($compile){
	var plugin_root = $juci.module("wifi").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/uci.wireless.schedule.edit.html", 
		scope: {
			schedule: "=ngModel"
		}, 
		controller: "uciWirelessScheduleEdit", 
		replace: true, 
		require: "^ngModel", 
		/*link: function(scope, elm, attrs, ctrl){
			scope.ctrl = ctrl; 
			ctrl.$validators.validate = function (modelValue, viewValue) {
				console.log(JSON.stringify(modelValue) +"-"+viewValue); 
				if (ctrl.$isEmpty(modelValue)) { // consider empty models to be valid
						return true;
				}
				
				return false;
			}
		}*/
	};  
}).controller("uciWirelessScheduleEdit", function($scope, gettext, $uci){
	$scope.selectedTimeFrame = []; 
	$scope.data = {}; 
	$scope.allTimeFrames = [
		{ label: gettext("Individual Days"), id: "inddays", value: [] }, 
		{ label: gettext("Every Day"), value: ["mon", "tue", "wed", "thu", "fri", "sat", "sun"] }, 
		{ label: gettext("Every Workday"), value: ["mon", "tue", "wed", "thu", "fri"] }, 
		{ label: gettext("All Weekend"), value: ["sat", "sun"] }
	]; 
	$scope.allDayNames = [
		{ label: gettext("Monday"), value: "mon" }, 
		{ label: gettext("Tuesday"), value: "tue" }, 
		{ label: gettext("Wednesday"), value: "wed" }, 
		{ label: gettext("Thursday"), value: "thu" }, 
		{ label: gettext("Friday"), value: "fri" }, 
		{ label: gettext("Saturday"), value: "sat" }, 
		{ label: gettext("Sunday"), value: "sun" }
	]; 
	$scope.selectedTimeFrame = $scope.allTimeFrames[0].value; 
	function update_time(){
		try {
			function split(value) { return value.split(":").map(function(x){ return Number(x); }); };
			var from = split($scope.data.timeFrom);
			var to = split($scope.data.timeTo); 
			if(from[0] >= 0 && from[0] < 24 && to[0] >= 0 && to[0] < 24 && from[1] >= 0 && from[1] < 60 && to[1] >= 0 && to[1] < 60){
				if((from[0]*60+from[1]) < (to[0]*60+to[1])) {
					$scope.schedule.time.value = ("0"+from[0]).slice(-2)+":"+("0"+from[1]).slice(-2)+"-"+("0"+to[0]).slice(-2)+":"+("0"+to[1]).slice(-2); 
				} else {
					console.log("'from' value must be less than 'to' value"); 
				}
			} else {
				console.log("invalid time"); 
			}
		} catch(e) {}
	}
	$scope.$watch("data.timeFrom", function(){
		update_time(); 
	}, true); 
	$scope.$watch("data.timeTo", function(){
		update_time(); 
	}, true);
	$scope.$watch("schedule", function(value){
		if(value == undefined) return; 
		var parts = value.time.value.split("-");//.map(function(x){ return x.split(":"); }); 
		$scope.data.timeFrom = parts[0]; 
		$scope.data.timeTo = parts[1]; 
		console.log("New model" + value.time.value); 
	}); 
	$scope.onChangeDays = function(){ 
		//$scope.schedule.days.value.splice(0,$scope.schedule.days.value.length); 
		$scope.schedule.days.value = $scope.selectedTimeFrame; 
		//console.log($scope.selectedTimeFrame); 
		//Object.assign($scope.schedule.days.value, $scope.selectedTimeFrame); 
	}
}); 
