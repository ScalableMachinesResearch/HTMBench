#!/bin/bash
set -x
MEMCACHED=$(pwd)/memcached/build_rtm/bin/memcached
MCPERF=$(pwd)/twemperf/install/bin/mcperf
#echo $HTM_TRETRY
$MEMCACHED -u memcached-user -t ${THREADS} > output.txt 2>&1 &

sleep 2

$MCPERF --linger=0 --timeout=5 --conn-rate=1000 --call-rate=1000 --num-calls=10 --num-conns=10000 --sizes=u1,16 > mcperf.output 2>&1
cat mcperf.output

killall $MEMCACHED

sleep 2

