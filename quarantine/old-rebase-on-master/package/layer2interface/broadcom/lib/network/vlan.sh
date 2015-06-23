#!/bin/sh

. /usr/share/libubox/jshn.sh
. /lib/network/ebtables.sh

removeall_vlandevices()
{
	local vif
	local i

	for i in `ls /proc/sys/net/ipv4/conf`; do
		case "$i" in 
			[eap][t][mh][0-9].v1)
			;;
			[eap][t][mh][0-9].1)
			;;
			[eap][t][mh][0-9].[v0-9]*)
				vlanctl --if-delete $i
			;;
		esac
	done
}

removevlan()
{
	vlanctl --if-delete $1
}

ifvlanexits()
{
	local vif=$1
	local i

	for i in `ls /proc/sys/net/ipv4/conf`; do
		if [ "$i" == "$vif" ]; then
			return 1 
		fi
	done
	return 0
}

ifbaseexists ()
{
	local if=$1

	if [ -d /sys/class/net/$if ]; then
		ifcarrier="/sys/class/net/$if/carrier"
		if [ -f $ifcarrier ] && [ "$(cat $ifcarrier)" == "1" ]; then
			return 1
		else
			json_load "$(devstatus "$if")"
			json_get_var link link
			if [ "$link" == "1" ]; then
				return 1
			fi
		fi
	fi
	return 0
}

addbrcmvlan ()
{
	local baseifname=$1
	local vlan8021p=$2
	local vlan8021q=$3
	local bridge=$4
	local ifname=$5
	#local ifname=$4
	#config_get baseifname $1 baseifname
	#config_get vlan8021p $1 vlan8021p
	#config_get vlan8021q $1 vlan8021q
	ifbaseexists $baseifname
	ret=$?
	if [ "$ret" -eq 1 ]; then
		ifvlanexits "$ifname"
		ret=$?
		echo "first ret=$ret"
		if [ "$ret" -eq 0 ]; then
			ifconfig $baseifname up
			echo "vlanctl --if-create $ifname"
			if [ "$bridge" -eq 1 ]; then
				vlanctl --if-create $baseifname $vlan8021q
				vlanctl --if $baseifname --set-if-mode-rg
				vlanctl --if $baseifname --tx --tags 0 --default-miss-drop
				vlanctl --if $baseifname --tx --tags 1 --default-miss-drop
				vlanctl --if $baseifname --tx --tags 2 --default-miss-drop
				# tags 0 tx
				vlanctl --if $baseifname --tx --tags 0 --filter-txif $ifname --push-tag --set-vid $vlan8021q 0 --set-pbits $vlan8021p 0 --rule-insert-before -1
				# tags 1 tx
				vlanctl --if $baseifname --tx --tags 1 --filter-txif $ifname --push-tag --set-vid $vlan8021q 0 --set-pbits $vlan8021p 0 --rule-insert-before -1
				# tags 2 tx
				vlanctl --if $baseifname --tx --tags 2 --filter-txif $ifname --push-tag --set-vid $vlan8021q 0 --set-pbits $vlan8021p 0 --rule-insert-before -1
				# tags 1 rx				
				vlanctl --if $baseifname --rx --tags 1 --filter-vid $vlan8021q 0 --pop-tag --set-rxif $ifname --rule-insert-before -1
				# tags 2 rx
				vlanctl --if $baseifname --rx --tags 2 --filter-vid $vlan8021q 0 --pop-tag --set-rxif $ifname --rule-insert-before -1
				ifconfig $ifname up
			else
				vlanctl --routed --if-create $baseifname $vlan8021q
				vlanctl --if $baseifname --set-if-mode-rg
				vlanctl --if $baseifname --tx --tags 0 --default-miss-drop
				vlanctl --if $baseifname --tx --tags 1 --default-miss-drop
				vlanctl --if $baseifname --tx --tags 2 --default-miss-drop
				# tags 0 tx
				vlanctl --if $baseifname --tx --tags 0 --filter-txif $ifname --push-tag --set-vid $vlan8021q 0 --set-pbits $vlan8021p 0 --rule-insert-before -1
				# tags 0 rx
				vlanctl --if $baseifname --rx --tags 0 --set-rxif $ifname --filter-vlan-dev-mac-addr 0 --drop-frame --rule-insert-before -1
				# tags 1 rx
				vlanctl --if $baseifname --rx --tags 1 --set-rxif $ifname --filter-vlan-dev-mac-addr 0 --drop-frame --rule-insert-before -1
				# tags 2 rx
				vlanctl --if $baseifname --rx --tags 2 --set-rxif $ifname --filter-vlan-dev-mac-addr 0 --drop-frame --rule-insert-before -1
				# tags 1 rx 
				vlanctl --if $baseifname --rx --tags 1 --filter-vlan-dev-mac-addr 1 --filter-vid $vlan8021q 0 --pop-tag --set-rxif $ifname --rule-insert-before -1	 
				# tags 2 rx
				vlanctl --if $baseifname --rx --tags 2 --filter-vlan-dev-mac-addr 1 --filter-vid $vlan8021q 0 --pop-tag --set-rxif $ifname --rule-insert-before -1
				ifconfig $ifname up
			fi
		fi
	fi
}


brcm_virtual_interface_rules ()
{
	local baseifname=$1
	local ifname=$2
	local bridge=$3
	
	echo '1' > /proc/sys/net/ipv6/conf/$baseifname/disable_ipv6
	ifconfig $baseifname up
	if [ "x$bridge" = "x" ]; then                                                                                                                         
	  bridge=0                                                                                                                                              
        fi
      
	if [ "$bridge" -eq 1 ]; then
	  vlanctl --if-create-name $baseifname $ifname
	  create_ebtables_bridge_rules  
	else
	  vlanctl --routed --if-create-name  $baseifname $ifname
	fi
	#set default RG mode
	vlanctl --if $baseifname --set-if-mode-rg
	#Set Default Droprules
	vlanctl --if $baseifname --tx --tags 0 --default-miss-drop
	vlanctl --if $baseifname --tx --tags 1 --default-miss-drop
	vlanctl --if $baseifname --tx --tags 2 --default-miss-drop
	vlanctl --if $baseifname --tx --tags 0 --filter-txif $ifname --rule-insert-before -1
	
	if [ "$bridge" -eq 1 ]; then
		
		# tags 1 tx
		vlanctl --if $baseifname --tx --tags 1 --filter-txif $ifname --rule-insert-before -1
		# tags 2 tx
		vlanctl --if $baseifname --tx --tags 2 --filter-txif $ifname --rule-insert-before -1
		# tags 0 rx
		vlanctl --if $baseifname --rx --tags 0 --set-rxif $ifname --rule-insert-last 
		# tags 1 rx
		vlanctl --if $baseifname --rx --tags 1 --set-rxif $ifname --rule-insert-last 
		# tags 2 rx
		vlanctl --if $baseifname --rx --tags 2 --set-rxif $ifname --rule-insert-last 
	else
				
		# tags 1 rx
		vlanctl --if $baseifname --rx --tags 1 --set-rxif $ifname --filter-vlan-dev-mac-addr 0 --drop-frame --rule-insert-before -1
		# tags 2 rx
		vlanctl --if $baseifname --rx --tags 2 --set-rxif $ifname --filter-vlan-dev-mac-addr 0 --drop-frame --rule-insert-before -1
		# tags 0 rx 
		vlanctl --if $baseifname --rx --tags 0 --set-rxif $ifname --filter-vlan-dev-mac-addr 1 --rule-insert-before -1
	fi
	
	ifconfig $ifname up
	ifconfig $ifname multicast	 
}


