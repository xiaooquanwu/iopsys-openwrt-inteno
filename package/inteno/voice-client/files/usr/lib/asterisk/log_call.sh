#!/bin/sh

#######################################################
# log_call
#
# Adds a row to the call log file. Will truncate file
# at 100 rows. Some very basic concurrency is supported.
#
# Usage:
# log_call.sh Direction From To [Note]
#
# Examples:
# ./log_call Incoming 07012345678 07876543210
# ./log_call Outgoing 07876543210 07012345678 Blocked
#####################################################

# Check that there is a direction specified
if [ -z "$1" ] ; then
	exit 1
fi

# Check that there is a calling  number specified
if [ -z "$2" ] ; then
	exit 2
fi

# Check that there is a called number specified
if [ -z "$3" ] ; then
	exit 2
fi

direction=$1

# Remove any suffix starting with an underscore
from=$(echo $2 | sed 's/_.*//')
to=$(echo $3 | sed 's/_.*//')

note=$4

logfile="/var/call_log"
tempfile=$(mktemp)
now=$(date)

for i in 1 2 3 4 5 ; do
	if mkdir /var/lock/log_call.lck ; then
		echo "$now;$direction;$from;$to;$note" >> $tempfile
		head -n99 $logfile >> $tempfile
		mv $tempfile $logfile
		rmdir /var/lock/log_call.lck
		break
	fi
	sleep 1
done

