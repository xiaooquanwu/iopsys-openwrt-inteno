#!/bin/sh
#
# Copyright (C) 2013-2014 Anyfi Networks AB.
# Anyfi.net setup functions for Broadcom wl driver.

# Get monitor name interface based for a device.
# anyfi_dev_monitor_name <device>
anyfi_broadcom_name_monitor() {
	local device="$1"

	# Map wlX => radiotapX
	echo "$device" | sed 's/^.*\([0-9]\)$/radiotap\1/'
}

# Get BSS index of the last wl interface that is used (=up)
# wl0.2 => 2
# wl0   => (empty)
anyfi_broadcom_get_wlindex() {
	ifconfig | grep -o "^$1\..." | cut -d'.' -f2 | sort -n | tail -n 1
}

# Allocate virtual Wi-Fi interfaces for anyfid.
# anyfi_broadcom_alloc_iflist <device> <bssids>
anyfi_broadcom_alloc_iflist() {
	local device="$1"
	local bssids="$2"
	local count=0
	local wlindex num

	# Enable MBSS mode if not already enabled
	if [ "$(wlctl -i $device mbss)" = 0 ]; then
		wlctl -i $device down
		wlctl -i $device mbss 1
		wlctl -i $device up
	fi

	wlindex=$(anyfi_broadcom_get_wlindex $device)

	# Create WLAN interfaces and let the driver assign the BSSIDs
	for num in $(seq $bssids); do
		local idx=$(($wlindex + $num))
		local wlif=$device.$idx

		# Do the 'wlctl' dance to make the driver assign proper BSSIDs
		wlctl -i $device bss -C $idx up > /dev/null || break
		wlctl -i $device ssid -C $idx "dummy" > /dev/null
		wlctl -i $device bss -C $idx up > /dev/null
		wlctl -i $device bss -C $idx down > /dev/null
		wlctl -i $device ssid -C $idx "" > /dev/null

		local bssid=$(wlctl -i $wlif cur_etheraddr | cut -d' ' -f2)
		ifconfig $wlif hw ether $bssid > /dev/null
		count=$(($count + 1))
	done

	[ "$count" -gt 0 ] && echo $device.$(($wlindex + 1))/$count
}

# Release virtual Wi-Fi interfaces allocated for anyfid.
# anyfi_broadcom_release_iflist <device>
anyfi_broadcom_release_iflist() {
	true
}

# Allocate a monitor interface for anyfid.
# anyfi_broadcom_alloc_monitor <device>
anyfi_broadcom_alloc_monitor() {
	local device="$1"
	local monitor=$(anyfi_broadcom_name_monitor $device)

	wlctl -i $device monitor 0 || return 1
	wlctl -i $device monitor 3 || return 1
	ifconfig $monitor down || return 1
	ifconfig $monitor up || return 1
	echo $monitor
}

# Release the monitor interface for anyfid.
# anyfi_broadcom_release_monitor <device>
anyfi_broadcom_release_monitor() {
	local device="$1"
	local monitor=$(anyfi_broadcom_name_monitor $device)

	ifconfig $monitor down 2> /dev/null
	wlctl -i $device monitor 0 2> /dev/null
}
