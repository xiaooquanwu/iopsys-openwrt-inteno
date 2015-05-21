$juci.module("internet")
    .controller("InternetDNSPageCtrl", function ($scope, $uci, $log) {

        $scope.providers = ["dyndns.org"];

        $log.info("uci", $uci);

        $uci.sync(["network","ddns"])
            .done(function () {
                if ($uci.network && $uci.network.wan) {
                    $log.debug("uci wan", $uci.network.wan);
                    $scope.wan = $uci.network.wan;
                    if ($scope.wan.dns && $scope.wan.dns.value.length == 0) {
                        $scope.wan.dns.value = ["",""]; // by default there are 2 DNS addresses
                    }
                } else {
                    $log.error("wan network not available on box");
                    // TODO show error message
                }
                if ($uci.ddns && $uci.ddns.myddns) {
                    $log.debug("uci myddns", $uci.ddns.myddns);
                    $scope.ddns = $uci.ddns.myddns;
                } else {
                    $log.error("ddns not available on box");
                    // TODO show error message
                }
            }).fail(function () {
                $log.error("Could not sync network or ddns settings!");
            })
            .always(function () {
                $scope.$apply();
            });

        $scope.$watch("wan.dns.value", function(value){
            if (value) {
                $uci.network.wan.dns.value = value;
            }
        }, true);

        $scope.onApply = function () {
            $scope.busy = 1;
            $uci.save().done(function () {
                $log.info("Settings saved!");
            }).fail(function () {
                $log.error("Could not save internet settings!");
            }).always(function () {
                $scope.$apply();
                $scope.busy = 0;
            });
        };

        $scope.onChangeProvider = function (item) {
            $uci.ddns.myddns.service_name = item;
        };

    });
