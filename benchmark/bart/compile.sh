#!/bin/bash

if [ "$#" -ne 1 ]; then
	echo "$0 [origin/rtm/all/clean]"
	exit 0
fi

if [ "$1" == "origin" ] || [ "$1" == "all" ]; then
	make allclean
	rm -rf build_origin
	make && make nufft && make install PREFIX=$(pwd)/build_origin &&\
	mkdir -p data &&\
	build_origin/bin/bart traj -x4096 -y4096 data/traj.ra &&\
	build_origin/bin/bart phantom data/shepplogan.ra -x 4096
fi

if [ "$1" == "rtm" ] || [ "$1" == "all" ]; then
	make allclean
	rm -rf build_rtm
	make RTM=yes && make nufft RTM=yes &&\
	make install RTM=yes PREFIX=$(pwd)/build_rtm &&\
	mkdir -p data &&\
        build_rtm/bin/bart traj -x4096 -y4096 data/traj.ra &&\
        build_rtm/bin/bart phantom data/shepplogan.ra -x 4096
fi

if [ "$1" == "clean" ]; then
	make allclean
	rm -rf build_rtm build_origin data
fi

