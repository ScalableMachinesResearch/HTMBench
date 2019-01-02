.PHONY: all memcached twemperf clean all-clean

CURRENT_DIR=$(shell pwd)

all: memcached twemperf


twemperf: twemperf/install/bin/mcperf

twemperf/install/bin/mcperf: twemperf/Makefile
	cd twemperf && make && make install
twemperf/Makefile : twemperf/configure
	cd twemperf && ./configure --prefix=$(CURRENT_DIR)/twemperf/install
twemperf/configure:
	cd twemperf && autoreconf -fvi

memcached:  memcached/memcached-debug

memcached/memcached-debug:
	cd memcached && ./compile.sh rtm


	
