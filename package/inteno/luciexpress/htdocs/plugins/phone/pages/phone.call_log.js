/*
 * juci - javascript universal client interface
 *
 * Project Author: Martin K. Schröder <mkschreder.uk@gmail.com>
 * 
 * Copyright (C) 2012-2013 Inteno Broadband Technology AB. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
 
$juci.module("phone")
.controller("PhoneCallLogPageCtrl", function($scope, $uci, gettext, $tr){ 
	$scope.phoneFilter = ""; 
	$scope.uniquePhones = [];
	$rpc.asterisk.call_log.list().done(function(res){
		var unique_phones = {}; 
		$scope.call_log = res.call_log.map(function(log){
			var date = new Date(log.time.replace(/CEST/g, "")); 
			var now = new Date(); 
			if(now.getDate() == date.getDate() && now.getMonth() == date.getMonth() && date.getFullYear() == now.getFullYear())
				log.date = $tr(gettext("Today")); 
			else 
				log.date = date.getDate() + ":"+date.getMonth()+":"+date.getFullYear(); 
			log.time = date.getHours()+":"+date.getMinutes(); 
			unique_phones[log.from] = true; 
			return log; 
		}); 
		$scope.phoneList = Object.keys(unique_phones).map(function(x){ return { from: x }; }); 
		$scope.phoneFilter = ""; 
		$scope.$apply(); 
	}); 
	$scope.onChangeFilter = function(item, model){
		$scope.phoneFilter = model.from; 
	}
	$scope.callListTbl = {
		rows: [
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "0171234568765", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "1123445451", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-right'></i>", "Today", "13:12", "123123123123", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-left'></i>", "Today", "13:12", "0171234568765", "Office", "1:13", "<input type='checkbox'/>"],
			["<i class='fa fa-arrow-right'></i>", "Today", "13:12", "123123123123", "Office", "1:13", "<input type='checkbox'/>"]
		]
	}; 
}); 
