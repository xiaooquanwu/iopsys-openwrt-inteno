$juci.module("core")
.directive("luciInputPort", function(){
	var plugin_root = $juci.module("core").plugin_root; 
	return {
		templateUrl: plugin_root+"/widgets/luci.input.port.html",
        restrict: 'E',
        replace: true,
        scope: {
            id: "@",
            name: "@",
            label: "@",
            model : "=",
            form: "=",
            labelClass : "@",
            inputOuterClass : "@",
            messageClass : "@",
            messageOffsetClass : "@"
        }
    };
})
.directive("validatePort", function(){
    var PORT_REGEX = /^\d{1,5}$/;
    return {
        require: 'ngModel',
        link: function(scope, elm, attrs, ctrl) {
            ctrl.$validators.validatePort = function(modelValue, viewValue) {
                if (ctrl.$isEmpty(modelValue)) { // consider empty models to be valid
                    return true;
                }
                if (PORT_REGEX.test(viewValue)) { // valid regex
                    if (viewValue > 0 && viewValue <= 65535) {
                        return true;
                    }
                }
                return false;
            }
        }
    }
});
