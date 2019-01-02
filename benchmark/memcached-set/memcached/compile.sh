#!/bin/bash
set -x
CURRENT=$(pwd)
TM_LIB=${TSX_ROOT}/lib/rtm

chmod u+x configure

if [ "$1" == "origin" ]; then
	./configure --prefix=$CURRENT/build_origin
	sed -i 's/-O2/-O3/g' Makefile
	make clean
	make -j && make install
	mv memcached build_origin/
fi

if [ "$1" == "rtm" ]; then 
	./configure --prefix=$CURRENT/build_rtm
	make clean
	./makefile_editor.py Makefile Makefile.rtm "CPPFLAGS+=-DRTM -I$TM_LIB" "INCLUDES+=-I$TM_LIB" "CFLAGS+=-I$TM_LIB -pthread" "LDFLAGS+=-L$TM_LIB" "LIBS+=-lrtm"
	mv Makefile.rtm Makefile
	sed -i 's/-O2/-O3/g' Makefile
	cd $TSX_ROOT/lib
	make CC=gcc
	cd $CURRENT
	make -j && make install
	#mv memcached build_rtm/
fi
if [ "$1" == "clean" ]; then
	make clean
	rm -rf build_rtm build_origin
fi
