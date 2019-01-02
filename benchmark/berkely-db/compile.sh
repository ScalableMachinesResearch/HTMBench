#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "$0 [rtm/clean]"
	exit 0
fi

if [ "$1" == "rtm" ] ; then
	make clean
	make
fi

if [ "$1" == "clean" ]; then
	make all-clean
fi

