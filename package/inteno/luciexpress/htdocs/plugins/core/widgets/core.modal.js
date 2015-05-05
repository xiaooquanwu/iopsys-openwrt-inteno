$juci.module("core")
.directive('modal', function () {
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root + "/widgets/core.modal.html", 
		restrict: 'E',
		transclude: true,
		replace:true,
		scope: {
			acceptLabel: "@",
			dismissLabel: "@",
			onAccept: "&",
			onDismiss: "&",
			ngShow: "=",
            title: "@",
            hideCloseBtn : "@",
            formIncluded : "@"
		}, 
		controller: "ModalController", 
		link: function postLink(scope, element, attrs) {
            scope.element = element;
			scope.$watch("ngShow", function(value){
				if(value == true)
					$(element).modal('show');
				else
					$(element).modal('hide');
			});

			$(element).on('shown.bs.modal', function(){
				scope.$apply(function(){
					scope.$parent[attrs.ngShow] = true;
				});
			});

			$(element).on('hidden.bs.modal', function(){
				scope.$apply(function(){
					scope.$parent[attrs.ngShow] = false;
				});
			});
		}
	};
})
.controller('ModalController', function($scope) {
	/*$scope.onDismiss = function(){
		console.log("Modal Dismissed"); 
	}
	$scope.onAccept = function(){
		console.log("Modal accepted!"); 
	}*/
	/*console.log("controller: "+$scope.ngShow); 
	$scope.$watch("ngShow", function(value){
		console.log("visible: "+$scope.ngShow); 
		if(value == true)
			$($scope.element).modal('show');
		else
			$($scope.element).modal('hide');
	});*/
});
