#!/bin/sh

#######################################################
# RingingSchedule
#
# Checks if ringing is active for SIP account
# for the current time.
#
# Return value is 1 if ringing is active, 0 otherwise.
#
# Usage:
# ringing_schedule.sh SIPAccount
#
# Example:
# ./ringing_scheudle.sh sip0
#####################################################

. /lib/functions.sh

contains()
{
	string="$1"
	substring="$2"
	if test "${string#*$substring}" != "$string"
	then
		echo "1"
	else
		echo "0"
	fi
	return
}

match_rules()
{
	#Loop over voice_clients sections, look for ringing_schedules
	local ___type=ringing_schedule
	local section cfgtype
	for section in ${CONFIG_SECTIONS}; do
		config_get cfgtype "$section" TYPE
		[ -n "$___type" -a "x$cfgtype" != "x$___type" ] && continue
		
		#Found ringing_schedule
		local sip_service_provider
		local days
		local time
		
		config_get sip_service_provider $section sip_service_provider
		config_get days $section days
		config_get time $section time
		
		#Match SIP Account		
		if [ "$sip_service_provider" != "$2" ]; then
			continue
		fi

		#Match current day
		days_matched=$(contains "$days" $3)
		if [ $days_matched != 1 ]; then
			continue
		fi

		#Match current time, split string of format 'HH:MM HH:MM'
		start_hour=${time:0:2}
		start_minute=${time:3:2}
		end_hour=${time:6:2}
		end_minute=${time:9:2}
		if [ $4 -lt $start_hour -o $4 -eq $start_hour -a $5 -lt $start_minute ] ; then
			continue
		fi

		if [ $4 -gt $end_hour -o $4 -eq $end_hour -a $5 -gt $end_minute ] ; then
			continue
		fi

		#Matched a ringing_schedule rule
		echo $1
		return 
	done
	
	echo "-1"
}

main()
{
	config_load voice_client
	
	local status
	local enabled
	local syslog
	
	#Return immediately if no SIPAccount is given
	if [ "$1" == "" ] || [ -z "$1" ]; then
		echo 1
		return
	fi
	
	#Return immediately if feature is disabled
	config_get status RINGING_STATUS status
	if [ "$status" == "0" ] || [ -z "$status" ]; then
		echo 1
		return
	fi

	#Check if ringing is enabled or disabled for the configured periods
	config_get enabled RINGING_STATUS enabled

	#Get current day and time
	day=$(/bin/date "+%A")
	hour=$(/bin/date "+%H")
	minute=$(/bin/date "+%M")

	#Check if configured periods are matched
	result=$(match_rules $enabled $1 $day $hour $minute)

	#If no rule matched, default is inverse of enabled
	if [ $result -lt 0 ]; then
		if [ $enabled -eq 0 ]; then
			result=1
		else
			result=0
		fi
	fi
	echo $result
}

result=$(main $1)
echo $result
