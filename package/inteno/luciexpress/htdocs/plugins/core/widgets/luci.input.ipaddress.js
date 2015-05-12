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
                model: "=",
                labelClass: "@"
            }
            //controller: "luciInputIpAddressController"
        };
    })
    .controller("luciInputIpAddressController", function($scope, $log) {
//$scope.data = "";
//$scope.$watch("data", function(){
//    $log.debug('model', $scope.model);
//    $log.debug('data', $scope.data);
//    if ($scope.model) $scope.model.value = $scope.data;
//}, true);
    });
