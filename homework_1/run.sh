#!/bin/bash
if [ $# -eq 0 ]; then
	echo "[FAIL] No arguments supplied"
else
	if [ $# -eq 2 ]; then
		if [ $1 = "-i" ]; then
			make IS_INTER=1 SRC=$2
		else
			echo "2" $1 $2
			make SRC=$1
		fi
	else
		make SRC=$1
	fi
fi
