$juci.module("core")
    .directive("luciInputPortRange", function () {
        var plugin_root = $juci.module("core").plugin_root;
        return {
            templateUrl: plugin_root + "/widgets/luci.input.port.range.html",
            restrict: 'E',
            replace: true,
            scope: {
                name: "@",
                label: "@",
                model: "=",
                form: "=",
                labelClass: "@",
                inputOuterClass: "@",
                messageClass: "@",
                messageOffsetClass: "@"
            },
            link: function() {

            }
        };
    })
    .directive("validatePortRange", function () {
        var PORT_REGEX = /^\d{1,5}$/;
        return {
            scope: {
                validatePortRange: '='
            },
            require: 'ngModel',
            restrict: 'A',
            link: function (scope, el, attrs, ctrl) {
                ctrl.$validators.validatePortRange = function (modelValue, viewValue) {
                    console.log('ctrl', ctrl);
                    console.log('scope', scope);
                    console.log('scope model', scope.model);
                    console.log('model value', modelValue);
                    console.log('view value', viewValue);
                    if (ctrl.$isEmpty(modelValue)) { // consider empty models to be valid
                        console.log('empty model');
                        ctrl.$setValidity('invalid', true);
                        return true;
                    }
                    if (PORT_REGEX.test(viewValue)) { // valid regex
                        console.log('view value = ' + viewValue);
                        if (viewValue <= 0 || viewValue > 65535) {
                            ctrl.$setValidity('invalid', false);
                            return false;
                        } else {
                            // Check port range
                            var startPort = '';
                            var endPort = '';
                            if (attrs.name === "startPort") {
                                startPort = viewValue;
                                endPort = getEndPortValueFromModel(modelValue);
                                if (endPort == 0 ) {
                                    ctrl.$setValidity('invalid', true);
                                    return true;
                                }
                            } else if (attrs.name === "endPort"){
                                startPort = getStartPortValueFromModel(modelValue);
                                endPort = viewValue;
                                if (startPort == 0 ) {
                                    ctrl.$setValidity('invalid', true);
                                    return true;
                                }
                            }
                            if (!startBeforeEnd(startPort, endPort)) {
                                ctrl.$setValidity('startgreaterthanend', false);
                                return false;
                            } else {

                                //attrs.ngModel = buildSingleFieldRangeForModel();
                                ctrl.$setValidity('invalid', true);
                                ctrl.$setValidity('startgreaterthanend', true);
                                return true;
                            }
                        }
                    } else {
                        ctrl.$setValidity('invalid', false);
                        return false;
                    }
                };
            }
        };

        function getStartPortValueFromModel(modelValue) {
            if (modelValue) {
                var range = modelValue.split('-');
                if (range.length > 0) {
                    return range[0];
                }
            }
            return 0;
        }

        function getEndPortValueFromModel(modelValue) {
            if (modelValue) {
                var range = modelValue.split('-');
                if (range.length > 1) {
                    return range[1];
                }
            }
            return 0;
        }

        function buildSingleFieldRangeForModel(startPort, endPort) {
            return startPort + "-" + endPort;
        }

        function startBeforeEnd(startPort, endPort) {
            if (startPort < endPort) {
                return true;
            }
            return false
        }
    });

