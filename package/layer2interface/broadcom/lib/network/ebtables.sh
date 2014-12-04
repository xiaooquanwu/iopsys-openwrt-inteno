#!/bin/sh

remove_ebtables_bridge_rules ()
{
	ebtables -D FORWARD -p ip --ip-protocol 17 --ip-destination-port 68 -j SKIPLOG 2>/dev/null
	ebtables -D FORWARD -p ip --ip-destination 255.255.255.255 -j SKIPLOG 2>/dev/null
}

#bypass fap acceleration forwarding for dhcp in bridge mode
create_ebtables_bridge_rules ()
{

	ebtables -A FORWARD -p ip --ip-protocol 17 --ip-destination-port 68 -j SKIPLOG
	ebtables -A FORWARD -p ip --ip-destination 255.255.255.255 -j SKIPLOG
}

# is called when a Wifi SSID is enabled with wme, which automatically
# enables its QoS queues 
remove_ebtables_wme_rules ()
{
local wifi_int=$1
      ebtables -t nat -D POSTROUTING -o $wifi_int -p IPV4 -j wmm-mark 2>/dev/null
      ebtables -t nat -D POSTROUTING -o $wifi_int -p IPV6 -j wmm-mark 2>/dev/null
      ebtables -t nat -D POSTROUTING -o $wifi_int -p 802_1Q -j wmm-mark --wmm-marktag vlan 2>/dev/null 

}

add_ebtables_wme_rules ()
{
local wifi_int=$1
      ebtables -t nat -A POSTROUTING -o $wifi_int -p IPV4 -j wmm-mark >/dev/null
      ebtables -t nat -A POSTROUTING -o $wifi_int -p IPV6 -j wmm-mark >/dev/null
      ebtables -t nat -A POSTROUTING -o $wifi_int -p 802_1Q -j wmm-mark --wmm-marktag vlan >/dev/null
}
add_ebtables_default_arp ()
{
      ebtables -t nat -A POSTROUTING -j mark --mark-or 0x7 -p ARP >/dev/null
}
remove_ebtables_default_arp ()
{
      ebtables -t nat -D POSTROUTING -j mark --mark-or 0x7 -p ARP >/dev/null
}
