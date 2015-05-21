$juci.module("internet")
    .controller("InternetDNSPageCtrl", function ($scope, $uci, $log) {

        $scope.providers = ["dyndns.org"];

        $uci.sync("network")
            .done(function () {
                if ($uci.network && $uci.network.wan) {
                    $log.info("wan", $uci.network.wan);
                    $scope.wan = $uci.network.wan;
                    $log.debug('$scope.wan.dns ', $scope.wan.dns);
                    if ($scope.wan.dns && $scope.wan.dns.value.length == 0) {
                        $scope.wan.dns.value = ["",""]; // by default there are 2 DNS addresses
                    }
                } else {
                    $log.error("wan network not available on box");
                    // TODO show error message
                }
            }).fail(function () {
                $log.error("Could not sync network settings!");
            })
            .always(function () {
                $scope.$apply();
            });

        //$uci.sync("ddns")
        //    .done( function(){
        //        console.log("ddns", $uci.ddns);
        //    })
        //    .always( function() {
        //        $scope.$apply();
        //    });

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
            $scope.selectedConfig = item;
            $scope.error = "";
            $scope.loading = 1;


            //$rpc.uci.state({
            //	config: item.id
            //}).done(function(data){
            //	$scope.subsections = data.values;
            //	Object.keys($scope.subsections).map(function(k){
            //		$scope.subsections[k] = filterHiddenValues($scope.subsections[k]);
            //	});
            //	$scope.loading = 0;
            //	$scope.$apply();
            //}).fail(function(err){
            //	console.error("Could not retrieve data!", err);
            //	$scope.error("Could not retrieve data!");
            //   $scope.loading = 0;
            //	$scope.$apply();
            //});
        };

    });
