$juci.module("core")
    .directive("luciInputIpAddress", function () {
        var plugin_root = $juci.module("core").plugin_root;
        return {
            templateUrl: plugin_root + "/widgets/luci.input.ipaddress.html",
            restrict: 'E',
            replace: true,
            scope: {
                id: "@",
                label: "@",
                ngModel: "=",
                labelClass: "@"
            },
            require: "ngModel",
            controller: "luciInputIpAddressController"
        };
    })
    .controller("luciInputIpAddressController", function($scope, $log) {
        $scope.data = [];
        $scope.ipAddress = "";

        $scope.$watch("ngModel", function(value){
            if(value && value.split){
                var parts = value.split(".");
                $scope.data[0] = parts[0]||"";
                $scope.data[1] = parts[1]||"";
                $scope.data[2] = parts[2]||"";
                $scope.data[3] = parts[3]||"";
            } else {
                $scope.ipAddress = value;
            }
        });

        $scope.$watch("data", function(){
            $log.debug('data', $scope.data);
            var ipAddress = $scope.data.join('.');
            $log.debug('ip address', ipAddress);
            if ($scope.ngModel) $scope.ngModel = ipAddress;
        }, true);
    });
