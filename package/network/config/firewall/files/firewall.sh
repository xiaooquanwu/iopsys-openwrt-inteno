#!/bin/sh

. /lib/functions.sh


local used8080=0
local used8081=0
local used8008=0
local used8888=0
local lan_and_wan=0
local noRule=1

check_forward(){
	local src_dport src
	config_get src $1 src
	config_get src_dport $1 src_dport
	case $src_dport in
	"80")
		if [ $src = "wan" ]; then
			lan_and_wan=1
		fi
		;;
	"8080")
		used8080=1;;
	"8081")
		used8081=1;;
	"8008")
		used8008=1;;
	"8888")
		used8888=1;;
	esac
}
check_rule(){
	local target src dest_port hidden port
	port=$2
	config_get target $1 target
	config_get src $1 src
	config_get dest_port $1 dest_port
	config_get hidden $1 hidden
	if [ "$target" = "ACCEPT" ] && [ "$src" = "wan" ] && [ "$dest_port" = "80" ] && [ "$hidden" = "1" ];then
		noRule=0
		uci set firewall.$1.dest_port=$port
		uci commit firewall
	fi
}
check_includes_for_rule(){
	local path port
	port=$2
	config_get path $1 path
	temp_file_name="./$(basename $0).$$.tmp" 
	[ -z $path ] || [ -f $path ] && awk -e '/iptables/ && /-I zone_wan_input/ && /-p tcp/ && /--dport 80/ && /-j ACCEPT/ { gsub("--dport 80 ", "--dport '${port}' ")} { print}' $path > $temp_file_name && mv $temp_file_name $path
}
change_webserver_listen_port(){
	local port 
	port=$1
	if grep -q "\$SERVER\[\"socket\"\][[:space:]]*==[[:space:]]*\":80\"" /etc/lighttpd/lighttpd.conf
	then
		if ! grep -q "\$SERVER\[\"socket\"\][[:space:]]*==[[:space:]]\":$port\"" /etc/lighttpd/lighttpd.conf
		then
			cat >> /etc/lighttpd/lighttpd.conf <<EOF
#Listen on ipv4
\$SERVER["socket"] == ":$port" {
}

EOF
		fi 
	fi
	if grep -q "\$SERVER\[\"socket\"\][[:space:]]*==[[:space:]]*\"\[::\]:80\"" /etc/lighttpd/lighttpd.conf
	then
		if ! grep -q "\$SERVER\[\"socket\"\][[:space:]]*==[[:space:]]*\"\[::\]:$port\"" /etc/lighttpd/lighttpd.conf
		then
			cat >> /etc/lighttpd/lighttpd.conf <<EOF
#Listen on ipv6
\$SERVER["socket"] == "[::]:$port" {
        server.use-ipv6 = "enable"
}

EOF
		fi
	fi
}
change_webserver_listen_port_uci(){
	local port listenports
	port=$1
	listenports=$(uci -q get uhttpd.main.listen_http)
	for i in $listenports
	do
		if echo $i | grep -qw "0.0.0.0:80" 
		then
			if ! echo $listenports | grep -qw "0.0.0.0:"$port
			then
				uci -q add_list uhttpd.main.listen_http="0.0.0.0:"$port
			fi
		fi
		if echo $i | grep -qw "\[::\]:80" 
		then
			if ! echo $listenports | grep -qw "\[::\]:"$port 
			then
				uci -q add_list uhttpd.main.listen_http="[::]:"$port
			fi
		fi
	done
	uci -q commit uhttpd
}
http_port_management(){
	#load the config file
	config_load firewall
	#check all redirects if port 80 on wan is forwared and looks for free port
	config_foreach check_forward redirect
	#any free ports??
	[ "$((used8080 + used8081 + used8008 + used8888 ))" = "4" ] && return
	local port
	case "0" in
		$used8080)
			port="8080";;
		$used8081)
			port="8081";;
		$used8008)
			port="8008";;	
		$used8888)
			port="8888";;
	esac
	#looks for exsisting rule on port 80 and if it finds it moves it to $port
	[ "$lan_and_wan" = "1" ] && config_foreach check_rule rule $port
	#if no rule is found all includes are checked for rule and fixed
	[ "$noRule" = "1" ] && config_foreach check_includes_for_rule include $port
	#change the port the webserver listes on
	[ -f /etc/lighttpd/lighttpd.conf ] && change_webserver_listen_port $port
	change_webserver_listen_port_uci $port
}
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

