#!/bin/sh

#######################################################
# log_call
#
# Adds a row to the call log file. Will truncate file
# at 100 rows. Some very basic concurrency is supported.
#
# Usage:
# log_call.sh Direction Number [Note]
#
# Examples:
# ./log_call Incoming 07012345678
# ./log_call Outgoing 07876543210 Blocked
#####################################################

#Check that there is a direction specified
if [ -z "$1" ] ; then
	exit 1
fi

#Check that there is a number specified
if [ -z "$2" ] ; then
	exit 2
fi

logfile="/var/call_log"
tempfile=$(mktemp)
now=$(date)

for i in 1 2 3 4 5 ; do
	if mkdir /var/lock/log_call.lck ; then
		if ! [ -z $3 ] ; then
			echo "$now;$1;$2;$3" >> $tempfile
		else
			echo "$now;$1;$2" >> 	$tempfile
		fi
		head -n99 $logfile >> $tempfile
		mv $tempfile $logfile
		rmdir /var/lock/log_call.lck
		break
	fi
	sleep 1
done

