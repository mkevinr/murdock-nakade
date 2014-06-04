#!/bin/bash
RUN_SERVER=true;
RUN_CLIENT=true;
STATIONARY=false;
CONSTANT=false;
WILD=false;

POS_NOISE_COMMAND="--default-posnoise=5";
POS_NOISE_COMMAND="";
NO_POS_NOISE=false;
CONTROL_ENEMY=true;

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
	elif [ "$var" = "no_noise" ] || [ "$var" = "-no_noise" ] || [ "$var" = "--no_noise" ] || [ "$var" = "nonoise" ] || [ "$var" = "-nonoise" ] || [ "$var" = "--nonoise" ]
	then
		NO_POS_NOISE=false;
	elif [ "$var" = "no_enemy" ] || [ "$var" = "-no_enemy" ] || [ "$var" = "--no_enemy" ] || [ "$var" = "noenemy" ] || [ "$var" = "-noenemy" ] || [ "$var" = "--noenemy" ]
	then
		CONTROL_ENEMY=false;
	fi
done

if [ "$NO_POS_NOISE" = true ];
then
	POS_NOISE_COMMAND=""
fi;

SERVER_DIR=~/git/bzrflag-server
KALMAN_DIR=~/git/bzrflag/Kalman_lab

if [ "$RUN_SERVER" = true ];
then
	$SERVER_DIR/bin/bzrflag --world=$KALMAN_DIR/empty4.bzw --world-size=370 --red-port=50100 --green-port=50101 --purple-port=50102 --blue-port=50103 --red-tanks=1 --green-tanks=1 $POS_NOISE_COMMAND &
fi;

sleep 2

if [ "$RUN_CLIENT" = true ];
then
	if [ "$STATIONARY" = true ]
	then
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman;
	elif [ "$CONSTANT" = true ]
	then
		if [ "$CONTROL_ENEMY" = true ];
		then
			$KALMAN_DIR/Debug/Kalman_lab localhost 50101 constant &
		fi
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman;

	elif [ "$WILD" = true ]
	then
		if [ "$CONTROL_ENEMY" = true ];
		then
			$KALMAN_DIR/Debug/Kalman_lab localhost 50101 wild &
		fi
		$KALMAN_DIR/Debug/Kalman_lab localhost 50100 kalman
	fi		
fi
