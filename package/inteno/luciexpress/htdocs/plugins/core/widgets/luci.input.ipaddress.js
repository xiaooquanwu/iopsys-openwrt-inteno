$juci.module("core")
    .directive("luciInputIpAddress", function (gettext, $log) {
        var plugin_root = $juci.module("core").plugin_root;
        return {
            templateUrl: plugin_root + "/widgets/luci.input.ipaddress.html",
            restrict: 'E',
            scope: {
                id: "@",
                label: "@",
                ngModel: "=",
                labelClass: "@"
            },
            require: "^ngModel",
            link: function (scope, element, attrs, ctrl) {
                $log.debug("scope", scope);
                $log.debug("element", element);
                $log.debug("attrs", attrs);
                $log.debug("ctrl", ctrl);

                scope.data = [];

                console.log("MODEL VALUE: ", scope.ngModel);
                if(ctrl.ngModel && ctrl.ngModel.split){
                    var parts = value.split(".");
                    scope.data[0] = parts[0]||"";
                    scope.data[1] = parts[1]||"";
                    scope.data[2] = parts[2]||"";
                    scope.data[3] = parts[3]||"";
                }

                scope.$watch("data", function() {
                    $log.debug("data", scope.data);
                    var ipAddress = scope.data.join('.');
                    $log.debug('ip address', ipAddress);
                    scope.ngModel = ipAddress;
                }, true);
            }
        };
    });
