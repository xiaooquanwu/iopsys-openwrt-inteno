$juci.module("internet")
    .controller("InternetDNSPageCtrl", function ($scope, $uci, $log) {

        $scope.providers = ["dyndns.org"];

        $scope.dns = {
            primary: "",
            secondary: ""
        };

        $uci.sync("network")
            .done(function () {
                if ($uci.network && $uci.network.wan) {
                    $log.info("wan", $uci.network.wan);
                    $scope.wan = $uci.network.wan;
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

        $scope.$watch("dns.primary", function(value){
					if(!$uci.network.wan) return; 
            $log.debug("dns primary = " + value);
            $log.debug("$uci.network.wan.dns.value", $uci.network.wan.dns.value);
            if ($uci.network.wan.dns.value) {
                $uci.network.wan.dns.value = [value];
            } else {
                $uci.network.wan.dns.value = [value, ""];
            }
        });

        $scope.$watch("dns.secondary", function(value){
					if(!$uci.network.wan) return; 
            $log.debug("dns secondary = " + value);
            $log.debug("$uci.network.wan.dns.value", $uci.network.wan.dns.value);
            if ($uci.network.wan.dns.value) {
                $uci.network.wan.dns.value[1] = [value];
            } else {
                $uci.network.wan.dns.value = ["", value];
            }
        });

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
