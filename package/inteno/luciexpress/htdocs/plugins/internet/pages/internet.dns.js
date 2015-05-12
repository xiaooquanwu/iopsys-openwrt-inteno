$juci.module("internet")
    .controller("InternetDNSPageCtrl", function($scope, $uci){

        $uci.sync("network").done( function(){
             console.log($uci.network.wan.peerdns); });

        $scope.onApply = function(){
            $scope.busy = 1;
            $uci.save().done(function(){
                console.log("Settings saved!");
            }).fail(function(){
                console.error("Could not save internet settings!");
            }).always(function(){
                $scope.$apply();
                $scope.busy = 0;
            });
        }
    });
