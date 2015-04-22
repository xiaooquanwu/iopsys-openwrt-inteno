$juci.controller("PhoneCallLogPageCtrl", function($scope){
	$scope.phoneNumbers = [{
		label: "All Numbers"
	}, {
		label: "Home Main"
	}, {
		label: "Office"
	}, {
		label: "Guest Room"
	}]; 
	$scope.callListTbl = {
		rows: [
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "0171234568765", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "1123445451", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-right'></i>", "Today", "13:12", "123123123123", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "0171234568765", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-right'></i>", "Today", "13:12", "123123123123", "Office", "1:13", "<input type='checkbox'/>"]
		]
	}; 
}); 
