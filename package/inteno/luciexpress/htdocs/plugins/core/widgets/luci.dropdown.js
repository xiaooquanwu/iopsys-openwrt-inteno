$juci.module("core")
.directive('luciSelect', function ($compile) {
	return {
	restrict: 'E',
	scope: {
		selectedItem: '=ngModel', 
		items: '=ngItems',
		onChange: '&onChange',
		placeholder: '@placeholder',
		prefix: "@", 
		size: '=size'
	},
	link: function (scope, element, attrs) {
		var html = '';
		switch (attrs.type) {
			case "dropdown":
				html += '<div class="dropdown dropdown-toggle"  data-toggle="dropdown" ><a class="dropdown-toggle" role="button" data-toggle="dropdown"  href="javascript:;">{{((selectedItem||{}).label || placeholder) | translate}}<b class="caret"></b></a>';
				break;
			default:
				html += '<div class="btn-group"><button class="btn btn-default button-label {{size_class}}">{{selectedText | translate}}</button><button class="btn btn-default dropdown-toggle" data-toggle="dropdown"><span class="caret"></span></button>';
				break;
		}
		html += '<ul class="dropdown-menu"><li ng-repeat="item in itemList"><a tabindex="-1" data-ng-click="selectVal(item)" href="">{{item.label}}</a></li></ul></div>';
		element.append($compile(html)(scope));
		
		if(scope.size) 
			scope.size_class = "btn-"+scope.size; 
			
		scope.$watch("size", function(){
			scope.size_class = "btn-"+scope.size; 
		}); 
		scope.selectedText = scope.placeholder; 
		
		scope.$watch("items", function(value){
			if(value){
				scope.itemList = value.map(function(x){
					//alert(JSON.stringify(x)+" "+JSON.stringify(scope.selectedItem)); 
					if(typeof x == "object" && "value" in x){
						if(scope.selectedItem && scope.selectedItem.value == x.value)
							scope.selectedText = scope.selectedItem.label || scope.placeholder;
						else if(scope.selectedItem == x.value){
							scope.selectedText = x.label || scope.placeholder; 
						} 
						return { label: x.label, value: x.value }; 
					} else {
						if(scope.selectedItem == x){
							scope.selectedText = scope.selectedItem || scope.placeholder; 
						}
						return { label: x, value: x }; 
					}
				}); 
			}
		}); 
		scope.$watch("selectedItem", function(value){
			//if(value && "value" in value) scope.selectedText = value.value; 
			//else scope.selectedText = value || scope.placeholder; 
		}); 
		
		scope.selectVal = function (item) {
			if(!item) return; 
			switch (attrs.type) {
				case "dropdown":
					$('a.dropdown-toggle', element).html('<b class="caret"></b> ' + item.label);
					break;
				default:
					$('button.button-label', element).html(item.label);
					break;
			}
			scope.selectedItem = item.value; 
				
			scope.onChange(item);
		};
		//scope.selectVal(scope.selectedItem);
	}
	};
});
