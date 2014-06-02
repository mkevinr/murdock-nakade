#!/bin/bash
RUN_SERVER=true
RUN_CLIENT=true

for var in "$@"
do
	if [ "$var" = "noserver" ] || [ "$var" = "-noserver" ] || [ "$var" = "no_server" ] || [ "$var" = "--noserver" ] || [ "$var" = "-no_server" ] || [ "$var" = "--no_server" ]
	then
		RUN_SERVER=false
	elif [ "$var" = "noclient" ] || [ "$var" = "-noclient" ] || [ "$var" = "no_client" ] || [ "$var" = "--noclient" ] || [ "$var" = "-no_client" ] || [ "$var" = "--no_client" ]
	then
		RUN_CLIENT=false
	fi
done

SERVER_DIR=~/git/bzrflag-server
KALMAN_DIR=~/git/bzrflag/Kalman

if [ "$RUN_SERVER" = true ];
then
	$SERVER_DIR/bin/bzrflag --world=$SERVER_DIR/maps/empty3.bzw  --red-port=50100 --green-port=50101 --purple-port=50102 --blue-port=50103 --red-tanks=1 --green-tanks=1 --default-posnoise=5 &
fi;

sleep 2

if [ "$RUN_CLIENT" = true ];
then
	$KALMAN_DIR/Debug/Kalman &
fi
