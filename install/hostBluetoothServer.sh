#!/bin/bash

#Runs the server to listen for Bluetooth connections
# On every re-connect the process running rfcomm needs to 
# be restarted


# This is the signal coming from our process indicating the remote has stopped
#	communicating and we need to restart the bluetooth server
function do_for_SIGUSER1() 
{
	#Send the SIGINT to the current process
	echo $PID_RFCOMM_SERVER
	kill -INT $PID_RFCOMM_SERVER
	PID_RFCOMM_SERVER=0;
}

PID_RFCOMM_SERVER=0
trap 'do_for_SIGUSER1' 10

while true; 
do
	if (("$PID_RFCOMM_SERVER" < "1")); then
		sleep 5
		rfcomm listen rfcomm0 2 &
		export PID_RFCOMM_SERVER=$!
	fi
	sleep 1
done


