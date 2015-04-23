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
 
$juci.controller("PhoneCallLogPageCtrl", function($scope){
	$scope.phoneNumbers = [{
		label: "All Numbers"
	}, {
		label: "Home Main"
	}, {
		label: "Office"
	}, {
		label: "Guest Room"
	}]; 
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
