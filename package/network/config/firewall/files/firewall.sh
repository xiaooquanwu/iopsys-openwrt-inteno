#!/bin/sh

. /lib/functions.sh

rematch_duidip6()
{
	duid_to_ip6() {
		local duid ip6
		config_get duid "$1" duid
		if [ -n "$duid" ]; then
			ip6=$( grep "$duid" /tmp/hosts/odhcpd | head -1 | awk '{print$NF}')
			[ -n "$ip6" ] && uci set firewall."$1".dest_ip="$ip6"
		fi
	}
	config_foreach duid_to_ip6 rule
}

reindex_dmzhost()
{
	uci -q get firewall.dmzhost >/dev/null || return

	local enabled reload path cfgno
	enabled=$(uci -q get firewall.dmzhost.enabled)
	[ "$enabled" == "0" ] && return
	path=$(uci -q get firewall.dmzhost.path)
	uci delete firewall.dmzhost
	cfgno=$(uci -q add firewall include)
	uci rename firewall.$cfgno=dmzhost
	uci -q set firewall.dmzhost.path="$path"
	uci -q set firewall.dmzhost.reload="1"
}

reconf_parental()
{
	local parental
	reconf() {
		config_get_bool parental "$1" parental
		if [ "$parental" == "1" ]; then
			uci set firewall."$1".src="*"
			uci set firewall."$1".src_port=""
			uci set firewall."$1".dest="*"
			uci set firewall."$1".dest_port=""
			uci set firewall."$1".proto="tcpudp"
		fi
	}
	config_foreach reconf rule
}

update_enabled() {                              
	config_get name "$1" name             
	local section=$1
	echo "Name: $name, section: $section";
	if [ "$name" == "wan" ]; then
		if [ $(uci get firewall.settings.disabled) == "1" ]; then
			uci set firewall.$section.input="ACCEPT";
		else
			uci set firewall.$section.input="REJECT";
		fi                             
		uci commit firewall            
	fi                                               
}

firewall_preconf() {
	config_load firewall                                        
	config_foreach update_enabled zone
	rematch_duidip6
	reconf_parental
	reindex_dmzhost
	http_port_management
	uci commit firewall
}

