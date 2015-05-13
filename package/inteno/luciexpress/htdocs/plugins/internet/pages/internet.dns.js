$juci.module("internet")
	.controller("InternetDNSPageCtrl", function($scope, $uci){

        $scope.providers = ["dyndns.org"];

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
		};

		$scope.onChangeProvider = function(item){
			$scope.selectedConfig = item;
			$scope.error = "";
			$scope.loading = 1;
			$scope.subsections = {};


            $rpc.uci.state({
				config: item.id
			}).done(function(data){
				$scope.subsections = data.values;
				Object.keys($scope.subsections).map(function(k){
					$scope.subsections[k] = filterHiddenValues($scope.subsections[k]);
				});
				$scope.loading = 0;
				$scope.$apply();
			}).fail(function(err){
				console.error("Could not retrieve data!", err);
				$scope.error("Could not retrieve data!");
                $scope.loading = 0;
				$scope.$apply();
			});
		};

	});
