#!/bin/bash
RUN_SERVER=true
RUN_CLIENT=true
STATIONARY=false;
CONSTANT=false;
WILD=false;

for var in "$@"
do
	if [ "$var" = "noserver" ] || [ "$var" = "-noserver" ] || [ "$var" = "no_server" ] || [ "$var" = "--noserver" ] || [ "$var" = "-no_server" ] || [ "$var" = "--no_server" ]
	then
		RUN_SERVER=false
	elif [ "$var" = "noclient" ] || [ "$var" = "-noclient" ] || [ "$var" = "no_client" ] || [ "$var" = "--noclient" ] || [ "$var" = "-no_client" ] || [ "$var" = "--no_client" ]
	then
		RUN_CLIENT=false
	elif [ "$var" = "stationary" ] || [ "$var" = "-stationary" ] || [ "$var" = "--stationary" ]
	then
		STATIONARY=true;
	elif [ "$var" = "constant" ] || [ "$var" = "-constant" ] || [ "$var" = "--constant" ]
	then
		CONSTANT=true;
	elif [ "$var" = "wild" ] || [ "$var" = "-wild" ] || [ "$var" = "--wild" ]
	then
		WILD=true;
	fi
done

SERVER_DIR=~/git/bzrflag-server
KALMAN_DIR=~/git/bzrflag/Kalman_lab

if [ "$RUN_SERVER" = true ];
then
	$SERVER_DIR/bin/bzrflag --world=$KALMAN_DIR/empty3.bzw  --red-port=50100 --green-port=50101 --purple-port=50102 --blue-port=50103 --red-tanks=1 --green-tanks=1 --default-posnoise=5 &
fi;

sleep 2

if [ "$RUN_CLIENT" = true ];
then
	if [ "$STATIONARY" = true ]
	then
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman
	elif [ "$CONSTANT" = true ]
	then
		$KALMAN_DIR/Debug/Kalman_lab localhost 50101 constant &
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman
	elif [ "$WILD" = true ]
	then
		$KALMAN_DIR/Debug/Kalman_lab localhost 50101 wild &
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman
	fi		
fi
