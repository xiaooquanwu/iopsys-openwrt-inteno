$juci.module("core")
    .directive("luciInputPort", function () {
        var plugin_root = $juci.module("core").plugin_root;
        return {
            templateUrl: plugin_root + "/widgets/luci.input.port.html",
            restrict: 'E',
            replace: true,
            scope: {
                id: "@",
                name: "@",
                label: "@",
                model: "=",
                form: "=",
                labelClass: "@",
                inputOuterClass: "@",
                messageClass: "@",
                messageOffsetClass: "@"
            },
            controller: "luciInputPortController"
        };
    })
    .controller("luciInputPortController", function($scope, $log) {
        $scope.data = "";
        $scope.$watch("data", function(){
            $log.debug('model', $scope.model);
            $log.debug('data', $scope.data);
            if ($scope.model) $scope.model.value = $scope.data;
        }, true);
    });
