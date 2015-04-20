angular.module("luci").controller("StatsOverviewCtrl", function ($scope, $session, $rootScope, $rpc, gettextCatalog) {
	$scope.sysinfo = {}; 
	$scope.info = {}; 
	
	function tr(str){
		return gettextCatalog.getString(str); 
	}
	
	$scope.systemStatusTbl = {
		rows: [
			[tr("Hostname"), "N/A"],
			[tr("Model"), "N/A"],
			[tr("Firmware Version"), "N/A"],
			[tr("Kernel Version"), "N/A"],
			[tr("Local Time"), "N/A"],
			[tr("Uptime"), "N/A"],
			[tr("Load Average"), "N/A"]
		]
	}; 
	$scope.systemMemoryTbl = {
		rows: [
			[tr("Available"), '<luci-progress value="40" total="100" units="kB"></luci-progress>'],
			[tr("Cached"), "N/A"],
			[tr("Shared"), "N/A"]
		]
	}; 
	$scope.systemStorageTbl = {
		rows: [
			[tr("Root Usage (/)"), '<luci-progress value="40" total="100" units="kB"></luci-progress>'],
			[tr("Temporary Usage (/tmp)"), "N/A"]
		]
	}; 
	$scope.systemConnectionsTbl = {
		rows: [
			[tr("Active Connections"), '<luci-progress value="40" total="100" units="kB"></luci-progress>']
		]
	}; 
	$scope.systemDHCPLeasesTbl = {
		columns: [tr("Hostname"), tr("IPv4-Address"), tr("MAC-Address"), tr("Leasetime remaining")], 
		rows: [
			[tr("No active leases"), '', '', '']
		]
	}; 
	$scope.systemStationsTbl = {
		columns: [tr("MAC address"), tr("Signal"), tr("Noise"), tr("RX Rate"), tr("TX Rate")], 
		rows: [
			[tr("No active stations"), '', '', '', '']
		]
	}; 
	setTimeout(function(){
		$scope.systemMemoryTbl.rows = [
			["Available", '<luci-progress value="10" total="100" units="kB"></luci-progress>'],
			["Cached", "N/A"],
			["Shared", "N/A"]
		]; 
		$scope.$apply(); 
	}, 2000); 
	$rpc.router.info({}, function(stats){
		$scope.info = stats; 
		$scope.$apply(); 
	}); 
	$rpc.system.info({}, function(sysinfo){
		sysinfo.date = String(new Date(sysinfo.localtime));
		sysinfo.loadavg = (sysinfo.load[0] * 0.0001) + " " + (sysinfo.load[1] * 0.0001) + " "+ (sysinfo.load[2] * 0.0001);
		sysinfo.memory.free_pcnt = ((sysinfo.memory.free / sysinfo.memory.total) * 100); 
		sysinfo.memory.buffered_pcnt = (sysinfo.memory.buffered / sysinfo.memory.total) * 100; 
		sysinfo.memory.buffered_kb = sysinfo.memory.buffered / 1000; 
		sysinfo.memory.free_kb = sysinfo.memory.free / 1000; 
		sysinfo.memory.shared_kb = sysinfo.memory.shared / 1000; 
		$scope.sysinfo = sysinfo; 
		$scope.$apply(); 
	}); 
}); 
