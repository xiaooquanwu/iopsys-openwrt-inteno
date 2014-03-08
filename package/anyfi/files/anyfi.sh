#!/bin/sh
#
# Copyright (C) 2013-2014 Anyfi Networks AB.
#
# Introduction to Anyfi.net
# =========================
#
# Anyfi.net is an access point software extension that makes every Wi-Fi network
# accessible through every Wi-Fi access point. The user experience is completely
# seamless; if you have previously connected to a Wi-Fi network locally your
# device will automatically connect to the same network remotely whenever you
# come within range of another Anyfi.net enabled access point.
#
# Key features/properties:
#
#  * You don't need to install any software on the device; all the magic is on
#    the access point side. From the device point of view it's all standard
#    Wi-Fi.
#
#  * There is no manual registration step. When you connect your device to your
#    home Wi-Fi network it's automatically registered for remote access to that
#    network.
#
#  * Mobile devices "see" the home Wi-Fi SSID and are authenticated using the
#    same Wi-Fi security mechanism and credentials that are used to authenticate
#    devices locally. This is what makes authentication completely seamless.
#
#  * The Wi-Fi protocol is tunneled over IP from the visited access point to the
#    home access point. This means that the encryption keys are derived only in
#    the mobile device and in the trusted home access point, ensuring end-to-end
#    security even if an attacker is in control of the visited access point. You
#    can think of it as an automatic Wi-Fi based VPN of sorts.
#
#  * Since mobile devices connect to their home access points through a Wi-Fi
#    over IP tunnel they are assigned IP addresses by the home network. This
#    ensures seamless hand-over between visited access points and full traffic
#    traceability: you will not be blamed for something a "guest" has done.
#
#  * Only the MAC address of your device, the IP address of your access point
#    and information that's available in the beacon (SSID and similar) are ever
#    sent to the controller or a visited access point. No personally identifiable
#    information, and absolutely no authentication credentials ever leave your
#    system!
#
# Extensive documentation is available at http://anyfi.net/documentation.
#
# Overview of the integration
# ===========================
#
# Anyfi.net software consists of two user space daemons; the radio head daemon
# anyfid and the tunnel termination daemon myfid. They communicate with each other
# and a controller to orchestrate the seamless user experience.
#
# The radio head daemon anyfid listens on a monitor interface to detect when
# a mobile device has come within range, and dynamically allocates a virtual
# access point for that device. Any and all Wi-Fi traffic on the virtual access
# point is then relayed, through a Wi-Fi over IP tunnel, to the device's home
# network. The integration is responsible for creating the monitor interface and
# the pool of virtual access points; anyfid handles the rest.
#
# The tunnel termination daemon myfid sits on the other side of the
# Wi-Fi over IP tunnel, making it possible to connect remotely to the local
# Wi-Fi network. It is up to the integration to configure myfid to authenticate
# remote devices with the same credentials that are used to authenticate devices
# locally. This is key to the seamless user experience.
#
# Myfid is also responsible for registering in the controller the MAC address of
# devices that connect locally, so that they will automatically be offered
# remote access whenever they come close to an access point running anyfid.
# However, when the user changes the WPA passphrase all associations between
# previously connected devices and the local Wi-Fi network should be removed.
# The integration does so by starting myfid with the --reset flag in this case.
#
# Below is the integration logic in pseudo code. If you need to integrate
# Anyfi.net software in your own firmware build environment you can find step by
# step instructions at http://anyfi.net/integration.
#
# After enabling a Wi-Fi device:
#   IF an Anyfi.net controller is configured AND Anyfi.net is not disabled
#     ALLOCATE monitor interface and virtual access point pool for anyfid
#     START anyfid
#
#   FOREACH Wi-Fi interface of this device
#     IF an Anyfi.net controller is configured AND Anyfi.net is not disabled
#      GENERATE a config file for myfid
#
#      IF the WPA passphrase has changed
#        ADD the --reset flag to myfid arguments
#
#      START myfid on the Wi-Fi interface
#
# After disabling a Wi-Fi device:
#   STOP anyfid
#
#   FOREACH Wi-Fi interface of this device
#     STOP myfid on the Wi-Fi interface
#
# NOTE1: The integration provides remote access to all Wi-Fi interfaces on the
#        system that have "anyfi_enabled" set. Each interface will have
#        its own myfid daemon. There should however only be one anyfid daemon
#        per radio.
#
# NOTE2: On concurrent dual band routers each radio should have its own anyfid
#        daemon.
#
# UCI data model
# ==============
#
# anyfi.config.controller=FQDN
# anyfi.config.controller_key=PATH
# anyfi.config.optimizer_key=PATH
#
# wifi-device.anyfi_enabled={0|1}
# wifi-device.anyfi_iface=IFNAME
# wifi-device.anyfi_port=PORT
# wifi-device.anyfi_ceiling=PERCENT
# wifi-device.anyfi_floor=PERCENT
# wifi-device.anyfi_uplink=BPS
# wifi-device.anyfi_downlink=BPS
# wifi-device.anyfi_max_bssids=NUM
# wifi-device.anyfi_max_clients=NUM
#
# wifi-iface.anyfi_enabled={0|1}
# wifi-iface.anyfi_iface=IFNAME
# wifi-iface.anyfi_port=PORT
# wifi-iface.anyfi_autz_server=IP
# wifi-iface.anyfi_autz_secret=TEXT
# wifi-iface.anyfi_autz_port=PORT
# wifi-iface.anyfi_acct_server=IP
# wifi-iface.anyfi_acct_secret=TEXT
# wifi-iface.anyfi_acct_port=PORT

append ENABLE_HOOKS anyfi_enable
append DISABLE_HOOKS anyfi_disable

# Daemon run dir for temporary files
RUNDIR=/var/run

# Config file dir for persistent configuration files
CONFDIR=/etc

# Daemon log level 0-4 (debug only)
ANYFI_DEBUG=0

# Get the WLAN interface list for a WLAN device.
# anyfi_get_vifs <device>
anyfi_get_vifs() {
	local device=$1
	local vifs=""
	local cfg

	for cfg in $CONFIG_SECTIONS; do
		local type dev
		config_get type "$cfg" TYPE
		config_get dev "$cfg" device

		if [ "$type" = wifi-iface ] && [ "$dev" = "$device" ]; then
			append vifs "$cfg"
		fi
	done

	echo "$vifs"
}

# Get BSS index of the last wl interface that is used.
anyfi_get_wlindex() {
	ifconfig | grep -o "^$1\..." | cut -d'.' -f2 | sort -n | tail -n 1
}

# Setup monitor/vap interfaces.
# anyfi_dev_prepare <device>
anyfi_dev_prepare() {
	local device="$1"
	local type
	local nvaps idx

	config_get type "$device" type

	if [ "$type" != broadcom ]; then
		echo 'This Anyfi.net integration only supports broadcom driver. Please see' 1>&2
		echo 'http://anyfi.net/integration or contact support@anyfi.net for access' 1>&2
		echo 'to integrations for other proprietary drivers from Atheros, Ralink, etc.' 1>&2
		return 1
	fi

	# Enable MBSS mode if not already enabled
	if [ "$(wlctl -i $device mbss)" = 0 ]; then
		wlctl -i $device down
		wlctl -i $device mbss 1
		wlctl -i $device up
	fi

	local wlindex=$(anyfi_get_wlindex $device)
	local wlcount=$(($wlindex + 1))
	local nvaps idx

	# Create pool of virtual access point interfaces.
	# Use at least 4 virtual APs but try to keep number of beacons below 8.
	config_get anyfi_max_bssids "$device" nvaps
	if [ -n "$nvaps" ]; then
		[ $nvaps -lt 12 ] || nvaps=12
	elif [ $wlcount -lt 4 ]; then
		nvaps=$((8 - $wlcount))
	else
		nvaps=4
	fi

	# Create WLAN interfaces and let the driver assign the BSSIDs
	for cnt in $(seq $nvaps); do
		local idx=$(($wlindex + $cnt))
		local wlif=$device.$idx

		wlctl -i $device ssid -C $idx "dummy"
		wlctl -i $device bss -C $idx up
		wlctl -i $device bss -C $idx down
		wlctl -i $device ssid -C $idx ""

		local bssid=$(wlctl -i $wlif cur_etheraddr | cut -d' ' -f2)
		ifconfig $wlif hw ether $bssid 2> /dev/null
	done

	# Enable the Radiotap monitor interface
	wlctl -i $device monitor 0
	wlctl -i $device monitor 3
	ifconfig radiotap0 down
	ifconfig radiotap0 up
	IF_START=$device.$wlcount
	IF_COUNT=$nvaps
}

# Get the device channel.
# anyfi_dev_channel <device>
anyfi_dev_channel() {
	local device="$1"
	local hwmode channel

	config_get hwmode  "$device" hwmode
	config_get channel "$device" channel

	if [ "$channel" = auto -o "$channel" = 0 ]; then
		case "$hwmode" in
		auto)
			echo "auto"
			return;;

		*b*|*g*)
			echo "auto2"
			return;;

		*a*)
			echo "auto5"
			return;;
		esac
	fi

	echo "$channel"
}

# Start the Anyfi.net radio head daemon anyfid on a device.
# anyfi_dev_start <device> <controller> <controller-key>
anyfi_dev_start()
{
	local device="$1"
	local controller="$2"
	local controller_key="$3"

	# ALLOCATE monitor and pool of virtual access points
	if anyfi_dev_prepare $device; then
		local args=""

		local wan port floor ceiling uplink downlink clients
		config_get wan      "$device" anyfi_iface
		config_get port     "$device" anyfi_port
		config_get floor    "$device" anyfi_floor
		config_get ceiling  "$device" anyfi_ceiling
		config_get uplink   "$device" anyfi_uplink
		config_get downlink "$device" anyfi_downlink
		config_get clients  "$device" anyfi_max_clients

		# If there are no interfaces on this device then anyfid controls channel
		if [ $(anyfi_get_vifs $device | wc -w) -eq 0 ]; then
			args="$args --channel=$(anyfi_dev_channel $device)"
		fi

		[ -n "$wan"      ] && args="$args --bind-if=$wan"
		[ -n "$port"     ] && args="$args --bind-port=$port"
		[ -n "$floor"    ] && args="$args --floor=$floor"
		[ -n "$ceiling"  ] && args="$args --ceiling=$ceiling"
		[ -n "$uplink"   ] && args="$args --uplink=$uplink"
		[ -n "$downlink" ] && args="$args --downlink=$downlink"
		[ -n "$clients"  ] && args="$args --max-clients=$clients"
		[ -n "$controller_key" ] && args="$args --controller-key=$controller_key"

		# START anyfid
		echo "Starting Anyfi.net radio head daemon anyfid on $device"
		anyfid --accept-license -C "$controller" -B \
		       -P $RUNDIR/anyfid_$device.pid -v $ANYFI_DEBUG \
		       $args radiotap0 $IF_START/$IF_COUNT
	fi
}

# Generate the config file for myfid from UCI variables.
# anyfi_vif_gen_config <iface>
anyfi_vif_gen_config() {
	local iface="$1"

	local type net ssid enc key ifname uuid

	config_get type   "$iface" type
	config_get net    "$iface" network
	config_get ssid   "$iface" ssid
	config_get enc    "$iface" encryption
	config_get key    "$iface" key
	config_get ifname "$iface" ifname
	config_get uuid   "$iface" anyfi_uuid

	# Check basic settings before proceeding
	[ -n "$net" ] || [ -n "$ssid" ] || return 1

	local auth_proto auth_mode auth_cache group_rekey
	local ciphers wpa_ciphers rsn_ciphers
	local passphrase
	local auth_server auth_port auth_secret
	local autz_server autz_port autz_secret
	local acct_server acct_port acct_secret
	local acct2_server acct2_port acct2_secret
	local radius_nasid

	# Resolve explicit cipher overrides (tkip, ccmp or tkip+ccmp)
	case "$enc" in
	*+tkip+ccmp|*+tkip+aes)
		ciphers=tkip+ccmp
		;;

	*+ccmp|*+aes)
		ciphers=ccmp
		;;

	*+tkip)
		ciphers=tkip
		;;
	esac

	# Resolve authentication protocol (WPA or WPA2)
	case "$enc" in
	psk-mixed*|wpa-mixed*)
		auth_proto=wpa+rsn
		wpa_ciphers=$ciphers
		rsn_ciphers=$ciphers
		;;

	psk2*|wpa2*)
		auth_proto=rsn
		rsn_ciphers=$ciphers
		;;

	psk*|wpa*)
		auth_proto=wpa
		wpa_ciphers=$ciphers
		;;

	none)
		auth_proto=open
		if [ "$type" != any80211 ]; then
			echo "Anyfi.net does not allow open networks with $type driver" 1>&2
			return 1
		fi
		;;

	wep*)
		echo 'Anyfi.net does not provide remote access to WEP networks for security reasons' 1>&2
		return 1
		;;

	*)
		echo 'Unrecognized encryption type' 1>&2
		return 1
		;;
	esac

	# Resolve authenticator mode (PSK or 802.1X)
	case "$enc" in
	psk*)
		auth_mode=psk
		passphrase=$key
		[ -n "$passphrase"  ] || return 1
		;;

	wpa*)
		auth_mode=eap

		config_get auth_server "$iface"  auth_server
		config_get auth_port   "$iface"  auth_port
		config_get auth_secret "$iface"  auth_secret

		config_get acct_server "$iface"  acct_server
		config_get acct_port   "$iface"  acct_port
		config_get acct_secret "$iface"  acct_secret

		config_get auth_cache  "$iface"  auth_cache
		config_get group_rekey "$iface"  wpa_group_rekey
		config_get auth_nasid  "$iface"  auth_nasid

		[ -n "$auth_server" ] || return 1
		[ -n "$auth_secret" ] || return 1
		[ -n "$auth_port"   ] || auth_port=1812

		[ -n "$acct_server" -a -z "$acct_secret" ] && return 1
		[ -n "$acct_server" -a -z "$acct_port"   ] && acct_port=1813
		;;

	none)
		;;

	*)
		echo "Anyfi.net requires explicit 'encryption' configuration" 1>&2
		return 1
		;;
	esac

	# Read RADIUS authorization and additional accounting settings
	config_get autz_server  "$iface" anyfi_autz_server
	config_get autz_port    "$iface" anyfi_autz_port
	config_get autz_secret  "$iface" anyfi_autz_secret
	config_get acct2_server "$iface" anyfi_acct_server
	config_get acct2_port   "$iface" anyfi_acct_port
	config_get acct2_secret "$iface" anyfi_acct_secret

	# Generate common config file options
	cat <<EOF
ssid = '$ssid'
bridge = br-$net
auth_proto = $auth_proto
EOF

	# Generate dependent config file options
	[ -n "$auth_mode"    ] && echo "auth_mode = $auth_mode"
	[ -n "$auth_cache"   ] && echo "auth_cache = $auth_cache"
	[ -n "$rsn_ciphers"  ] && echo "rsn_ciphers = $rsn_ciphers"
	[ -n "$wpa_ciphers"  ] && echo "wpa_ciphers = $wpa_ciphers"
	[ -n "$group_rekey"  ] && echo "group_rekey = $group_rekey"
	[ -n "$uuid"         ] && echo "uuid = $uuid"
	[ -n "$ifname"       ] && echo "local_ap = $ifname"
	[ -n "$passphrase"   ] && echo "passphrase = '$passphrase'"
	[ -n "$auth_server"  ] && echo "radius_auth_server = $auth_server"
	[ -n "$auth_port"    ] && echo "radius_auth_port = $auth_port"
	[ -n "$auth_secret"  ] && echo "radius_auth_secret = $auth_secret"
	[ -n "$autz_server"  ] && echo "radius_autz_server = $autz_server"
	[ -n "$autz_port"    ] && echo "radius_autz_port = $autz_port"
	[ -n "$autz_secret"  ] && echo "radius_autz_secret = $autz_secret"
	[ -n "$acct_server"  ] && echo "radius_acct_server = $acct_server"
	[ -n "$acct_port"    ] && echo "radius_acct_port = $acct_port"
	[ -n "$acct_secret"  ] && echo "radius_acct_secret = $acct_secret"
	[ -n "$acct2_server" ] && echo "radius_acct2_server = $acct2_server"
	[ -n "$acct2_port"   ] && echo "radius_acct2_port = $acct2_port"
	[ -n "$acct2_secret" ] && echo "radius_acct2_secret = $acct2_secret"
	[ -n "$nasid"        ] && echo "radius_nas_id = $nasid"

	return 0
}

# Get a config value from a configuration file for myfid.
# anyfi_vif_config_get <file> <config>
anyfi_vif_config_get() {
	local file="$1"
	local key="$2"

	[ -e "$file" ] || return 1

	# Assume the format is exactly "key = value" (where value may or may not be in '')
	grep "$key = " $file | cut -d '=' -f2- | cut -b2- | sed -e "/^'.*'$/s/^'\\(.*\\)'$/\\1/"
}

# Start the Anyfi.net tunnel-termination daemon myfid on an interface.
# anyfi_vif_start <iface> <controller> <controller-key> <optimizer-key>
anyfi_vif_start()
{
	local iface="$1"
	local controller="$2"
	local controller_key="$3"
	local optimizer_key="$4"
	local ifname key

	config_get ifname "$iface" ifname
	config_get key	  "$iface" key

	local pid_file="$RUNDIR/myfid_${ifname:-$iface}.pid"
	local conf_file="$CONFDIR/myfid_${ifname:-$iface}.conf"
	local new_conf_file="$RUNDIR/myfid_${ifname:-$iface}.conf"

	# GENERATE a config file for myfid
	if (anyfi_vif_gen_config $iface) > $new_conf_file; then
		local port wan
		local args=""

		config_get wan  "$iface" anyfi_iface
		config_get port "$iface" anyfi_port

		[ -n "$wan"  ] && args="$args --bind-if=$wan"
		[ -n "$port" ] && args="$args --bind-port=$port"
		[ -n "$controller_key" ] && args="$args --controller-key=$controller_key"
		[ -n "$optimizer_key"  ] && args="$args --optimizer-key=$optimizer_key"

		if [ -e "$conf_file" ]; then
			# Get old passphrase
			local old_key="$(anyfi_vif_config_get $conf_file passphrase)"

			# ADD the --reset flag to myfid arguments if passphrase has changed
			[ "$key" == "$old_key" ] || args="$args --reset"
		fi

		# Update the myfid config file in flash only if needed
		if ! cmp -s $new_conf_file $conf_file ; then
			mv $new_conf_file $conf_file
		else
			rm -f $new_conf_file
		fi

		# START myfid
		echo "Starting Anyfi.net tunnel-termination daemon myfid on ${ifname:-$iface}"
		myfid --accept-license -C "$controller" -B -P $pid_file -v $ANYFI_DEBUG $args $conf_file
	fi
}

# Run from ENABLE_HOOKS.
anyfi_enable()
{
	local device="$1"
	local controller="$(uci get anyfi.config.controller)"
	local controller_key="$(uci get anyfi.config.controller_key)"
	local optimizer_key="$(uci get anyfi.config.optimizer_key)"
	local vif

	[ -n "$controller" ] || exit 0;

	# Reload the wireless config to get the assigned ifnames right
	config_load wireless

	# START anyfid
	if [ "$(config_get "$device" anyfi_enabled)" = 1 ]; then
		anyfi_dev_start $device $controller $controller_key
	fi

	# FOREACH Wi-Fi interface of this device
	for vif in $(anyfi_get_vifs $device); do
		if [ "$(config_get "$vif" anyfi_enabled)" = 1 ]; then
			anyfi_vif_start $vif $controller $controller_key $optimizer_key
		fi
	done
}

# Stop an Anyfi.net daemon gracefully.
# anyfi_stop_daemon <pidfile>
anyfi_stop_daemon() {
	local pidfile="$1"

	kill -TERM $(cat $pidfile)

	for t in $(seq 0 5); do
		[ -e $pidfile ] || return 0
		sleep 1
	done

	echo "Timeout waiting for daemon to exit and $pidfile to be removed" 1>&2
	kill -KILL $(cat $pidfile)
	rm -f $pidfile
	return 1
}

# Undo what anyfi_dev_prepare did.
# anyfi_dev_unprepare <device>
anyfi_dev_unprepare() {
	local device=$1

	# Take down monitor interface
	ifconfig radiotap0 down 2> /dev/null
	wlctl -i $device monitor 0 2> /dev/null
}

# Run from DISABLE_HOOKS.
anyfi_disable()
{
	local device="$1"
	local vif

	# Reload the wireless config to get the assigned ifnames right
	config_load wireless

	# STOP anyfid
	if [ -e $RUNDIR/anyfid_$device.pid ]; then
		echo "Stopping Anyfi.net radio head daemon anyfid on $device"
		anyfi_stop_daemon $RUNDIR/anyfid_$device.pid
		anyfi_dev_unprepare $device
	fi

	# FOREACH Wi-Fi interface of this device (with myfid running)
	for vif in $(anyfi_get_vifs $device); do
		local ifname pidfile

		config_get ifname "$vif" ifname
		pidfile="$RUNDIR/myfid_${ifname:-$vif}.pid"

		if [ -e $pidfile ]; then
			echo "Stopping Anyfi.net tunnel-termination daemon myfid on ${ifname:-$vif}"
			anyfi_stop_daemon $pidfile
		fi
	done
}
