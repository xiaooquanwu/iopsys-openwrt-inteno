$juci.module("status")
.controller("StatsOverviewCtrl", function ($scope, $session, $rootScope, $rpc, gettext, $tr) {
	$scope.sysinfo = {}; 
	$scope.info = {}; 
	
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
		columns: [gettext("Hostname"), gettext("IPv4-Address"), gettext("MAC-Address"), gettext("Leasetime remaining")], 
		rows: [
			[gettext("No active leases"), '', '', '']
		]
	}; 
	$scope.systemStationsTbl = {
		columns: [gettext("IPv4-Address"), gettext("MAC address"), gettext("Signal"), gettext("Noise"), gettext("RX Rate"), gettext("TX Rate")], 
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
				[$tr(gettext("Hostname")), info.system.name],
				[$tr(gettext("Model")), info.system.nvramver],
				[$tr(gettext("Firmware Version")), info.system.firmware],
				[$tr(gettext("Kernel Version")), info.system.kernel],
				[$tr(gettext("Local Time")), new Date(sys.localtime)],
				[$tr(gettext("Uptime")), info.system.uptime],
				[$tr(gettext("Load Average")), sys.load[0] + " " + sys.load[1] + " " + sys.load[2]]
			]; 
			$scope.systemExtStatusTbl.rows = [
				[$tr(gettext("Wifi")), (info.specs.wifi)?gettext("yes"):gettext("no")],
				[$tr(gettext("ADSL")), (info.specs.adsl)?gettext("yes"):gettext("no")],
				[$tr(gettext("VDSL")), (info.specs.vdsl)?gettext("yes"):gettext("no")],
				[$tr(gettext("Voice")), (info.specs.voice)?gettext("yes"):gettext("no")],
				[$tr(gettext("Voice Ports")), info.specs.voice_ports],
				[$tr(gettext("Ethernet Ports")), info.specs.eth_ports]
			]; 
			$scope.systemMemoryTbl.rows = [
				[$tr(gettext("Usage")), '<luci-progress value="'+Math.round((sys.memory.total - sys.memory.free) / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[$tr(gettext("Shared")), '<luci-progress value="'+Math.round(sys.memory.shared / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[$tr(gettext("Buffered")), '<luci-progress value="'+Math.round(sys.memory.buffered / 1000)+'" total="'+ Math.round(sys.memory.total / 1000) +'" units="kB"></luci-progress>'],
				[$tr(gettext("Swap")), '<luci-progress value="'+Math.round((sys.swap.total - sys.swap.free) / 1000)+'" total="'+ Math.round(sys.swap.total / 1000) +'" units="kB"></luci-progress>']
			];
			$scope.systemStorageTbl.rows = [
				[$tr(gettext("Root Usage (/)")), '<luci-progress value="'+Math.round(disk.root.used / 1000)+'" total="'+ Math.round(disk.root.total / 1000) +'" units="kB"></luci-progress>'],
				[$tr(gettext("Temporary Usage (/tmp)")), '<luci-progress value="'+Math.round(disk.tmp.used / 1000)+'" total="'+ Math.round(disk.tmp.total / 1000) +'" units="kB"></luci-progress>']
			]; 
			$scope.systemConnectionsTbl.rows = [
				[$tr(gettext("Active Connections")), '<luci-progress value="'+ conntrack.count +'" total="'+conntrack.limit+'"></luci-progress>']
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
					[$tr(gettext("No active leases")), '', '', '']
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
					[$tr(gettext("No active stations")), '', '', '', '', '']
				]; 
			}
			$scope.$apply(); 
		}, function(){
			setTimeout(function(){
				doUpdate();
			}, 5000); 
		}); 
	}
	doUpdate(); 
}); 
