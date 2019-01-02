#!/bin/bash


if [ "$1" == "rtm" ]; then
	./configure CPPFLAGS=-I$TSX_ROOT/lib/rtm LDFLAGS="-L$TSX_ROOT/lib/rtm -lrtm"
	make -j
fi

if [ "$1" == "clean" ]; then
	make distclean
fi
