$juci.module("core")
.directive('luciDropdown', function ($compile) {
	return {
		restrict: 'E',
		scope: {
				items: '=ngModel',
				doSelect: '&onChange',
				selectedItem: '=ngSelected',
				placeholder: '@placeholder'
		},
		link: function (scope, element, attrs) {
			var html = '';
			switch (attrs.type) {
					case "button":
							html += '<div class="btn-group"><button class="btn btn-default button-label">{{placeholder}}</button><button class="btn btn-default dropdown-toggle" data-toggle="dropdown"><span class="caret"></span></button>';
							break;
					default:
							html += '<div class="dropdown"><a class="dropdown-toggle" role="button" data-toggle="dropdown"  href="javascript:;">Dropdown<b class="caret"></b></a>';
							break;
			}
			html += '<ul class="dropdown-menu"><li ng-repeat="item in items"><a tabindex="-1" data-ng-click="selectVal(item)" href="">{{item.label}}</a></li></ul></div>';
			element.append($compile(html)(scope));
			scope.$watch("items", function(){
				if(scope.items){
					if(scope.items.length) scope.bSelectedItem = scope.items[0];
					for (var i = 0; i < scope.items.length; i++) {
							if (scope.items[i].id === scope.selectedItem) {
									scope.bSelectedItem = scope.items[i];
									break;
							}
					}
				}
			}); 
			scope.selectVal = function (item) {
				if(!item) return; 
				switch (attrs.type) {
						case "button":
								$('button.button-label', element).html(item.label);
								break;
						default:
								$('a.dropdown-toggle', element).html('<b class="caret"></b> ' + item.label);
								break;
				}
				scope.selectedItem = item;  
				scope.doSelect(item);
			};
			scope.selectVal(scope.bSelectedItem);
		}
	};
});
