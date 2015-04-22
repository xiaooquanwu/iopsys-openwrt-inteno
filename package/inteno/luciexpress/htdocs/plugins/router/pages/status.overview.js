$juci.module("router")
.controller("StatsOverviewCtrl", function ($scope, $session, $rootScope, $rpc, gettextCatalog) {
	$scope.sysinfo = {}; 
	$scope.info = {}; 
	
	function tr(str){
		return gettextCatalog.getString(str); 
	}
	
	$scope.systemStatusTbl = {
		rows: [["", ""]]
	}; 
	$scope.systemExtStatusTbl = {
		rows: [["", ""]]
	}; 
	$scope.systemMemoryTbl = {
		rows: [["", ""]]
	}; 
	$scope.systemStorageTbl = {
		rows: [["", ""]]
	}; 
	$scope.systemConnectionsTbl = {
		rows: [["", ""]]
	}; 
	$scope.systemDHCPLeasesTbl = {
		columns: [tr("Hostname"), tr("IPv4-Address"), tr("MAC-Address"), tr("Leasetime remaining")], 
		rows: [
			[tr("No active leases"), '', '', '']
		]
	}; 
	$scope.systemStationsTbl = {
		columns: [tr("IPv4-Address"), tr("MAC address"), tr("Signal"), tr("Noise"), tr("RX Rate"), tr("TX Rate")], 
		rows: []
	};
	var info = {};
	var sys = {};  
	var conntrack = {}; 
	var disk = {}; 
	var clients = {}; 
	var leases = {}; 
	function doUpdate(){
		async.parallel([
			function (cb){$rpc.router.info().done(function(res){info = res; cb();}).fail(function(res){cb();});},
			function (cb){$rpc.system.info().done(function(res){sys = res; cb();}).fail(function(res){cb();});},
			function (cb){$rpc.luci2.network.conntrack_count().done(function(res){conntrack = res; cb();}).fail(function(res){cb();});},
			function (cb){$rpc.luci2.system.diskfree().done(function(res){disk = res; cb();}).fail(function(res){cb();});},
			function (cb){$rpc.router.clients().done(function(res){clients = res; cb();}).fail(function(res){cb();});},
			function (cb){$rpc.luci2.network.dhcp_leases().done(function(res){leases = res.leases || []; cb();}).fail(function(res){cb();});}
		], function(err, next){
			$scope.systemStatusTbl.rows = [
				[tr("Hostname"), info.system.name],
				[tr("Model"), info.system.nvramver],
				[tr("Firmware Version"), info.system.firmware],
				[tr("Kernel Version"), info.system.kernel],
				[tr("Local Time"), new Date(sys.localtime)],
				[tr("Uptime"), info.system.uptime],
				[tr("Load Average"), sys.load[0] + " " + sys.load[1] + " " + sys.load[2]]
			]; 
			$scope.systemExtStatusTbl.rows = [
				[tr("Wifi"), (info.specs.wifi)?tr("yes"):tr("no")],
				[tr("ADSL"), (info.specs.adsl)?tr("yes"):tr("no")],
				[tr("VDSL"), (info.specs.vdsl)?tr("yes"):tr("no")],
				[tr("Voice"), (info.specs.voice)?tr("yes"):tr("no")],
				[tr("Voice Ports"), info.specs.voice_ports],
				[tr("Ethernet Ports"), info.specs.eth_ports]
			]; 
			$scope.systemMemoryTbl.rows = [
				[tr("Usage"), '<luci-progress value="'+Math.round((sys.memory.total - sys.memory.free) / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[tr("Shared"), '<luci-progress value="'+Math.round(sys.memory.shared / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[tr("Buffered"), '<luci-progress value="'+Math.round(sys.memory.buffered / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[tr("Swap"), '<luci-progress value="'+Math.round((sys.swap.total - sys.swap.free) / 1000)+'" total="'+ Math.round(sys.swap.total / 1000) +'" units="kB"></luci-progress>']
			];
			$scope.systemStorageTbl.rows = [
				[tr("Root Usage (/)"), '<luci-progress value="'+Math.round(disk.root.used / 1000)+'" total="'+ Math.round(disk.root.total / 1000) +'" units="kB"></luci-progress>'],
				[tr("Temporary Usage (/tmp)"), '<luci-progress value="'+Math.round(disk.tmp.used / 1000)+'" total="'+ Math.round(disk.tmp.total / 1000) +'" units="kB"></luci-progress>']
			]; 
			$scope.systemConnectionsTbl.rows = [
				[tr("Active Connections"), '<luci-progress value="'+ conntrack.count +'" total="'+conntrack.limit+'" units="Connections"></luci-progress>']
			]; 
			if(leases.length){
				$scope.systemDHCPLeasesTbl.rows = []; 
				leases.map(function(lease){
					var date = new Date(null);
					date.setSeconds(lease.expires); // specify value for SECONDS here
					var time = date.toISOString().substr(11, 8);
					$scope.systemDHCPLeasesTbl.rows.push(
						[lease.hostname, lease.ipaddr, lease.macaddr, time]
					);  
				}); 
			} else {
				$scope.systemDHCPLeasesTbl.rows = [
					[tr("No active leases"), '', '', '']
				]; 
			}
			if(Object.keys(clients).length){
				$scope.systemStationsTbl.rows = []; 
				Object.keys(clients).map(function(id){
					var cl = clients[id]; 
					$scope.systemStationsTbl.rows.push(
						[cl.ipaddr, cl.macaddr, 0, 0, cl.rx_rate || 0, cl.tx_rate || 0]
					); 
				}); 
			} else {
				$scope.systemStationsTbl.rows = [
					[tr("No active stations"), '', '', '', '', '']
				]; 
			}
			$scope.$apply(); 
		}); 
	}
	doUpdate(); 
	setInterval(doUpdate, 5000); 
}); 
