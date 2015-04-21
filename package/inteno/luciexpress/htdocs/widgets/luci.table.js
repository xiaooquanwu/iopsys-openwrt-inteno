angular.module("luci")
	.directive('dynamic', function ($compile, $interpolate) {
		return {
			restrict: 'A',
			replace: true,
			link: function (scope, element, attrs) {
				scope.$watch(attrs.dynamic, function(html) {
					element.html(html);
					$compile(element.contents())(scope);
				});
			}
		};
	})
	.directive("luciTable", function($compile){
		return {
			// accepted parameters for this tag
			scope: {
				data: "=", 
				columns: "=", 
				title: "=", 
				noequalize: "="
			}, 
			templateUrl: "widgets/luci.table.html", 
			replace: true, 
			controller: "TableControl",
			controllerAs: "ctrl",
			/*link: function(scope, element, attributes){
				 $compile('<luci-progress value="50" total="100"></luci-progress>')(scope, function(cloned, scope){ 
					 element.html(cloned); 
					}); 
				}*/
     };  
	}).controller("TableControl", function($scope){
		if(!$scope.data)
			$scope.data = {}; 
		// assign columns from passed argument if present
		if($scope.columns && $scope.columns.length){
			if(!$scope.data.hasOwnProperty("columns"))
				$scope.data.columns = []; 
			Object.assign($scope.data.columns, $scope.columns); 
		}
		// make all columns the same size
		$scope.cell_class = ""; 
		$scope.first_cell_class = ""; 
		var ncols = 0; 
		if($scope.data.rows && $scope.data.rows[0]){
			ncols = $scope.data.rows[0].length; 
		} else if($scope.data.columns){
			ncols = $scope.data.columns.length; 
		} else {
			ncols = 0; 
		}
		if(ncols){
			// try to extend first column if the division is not whole number 
			if(ncols < 12){
				var w = Math.round(12 / ncols); 
				$scope.cell_class = "col-md-"+w;
				if(((12 / ncols) - w) != 0){ 
					$scope.first_cell_class="col-md-"+(12 - w * (ncols - 1)); 
				} else {
					$scope.first_cell_class= $scope.cell_class; 
				}
			}
		} 
		$scope.cellClass = function(idx){
			if(idx == 0) return $scope.first_cell_class; 
			return $scope.cell_class; 
		}
	}); 
