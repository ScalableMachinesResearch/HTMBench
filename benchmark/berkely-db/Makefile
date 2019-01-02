.PHONY: all bdb bench all-clean

CURRENT_DIR = $(shell pwd)

all: bdb bench

bdb: build_unix/Makefile
	cd build_unix && make -j && make install

build_unix/Makefile:
	cd build_unix && \
	../dist/configure --enable-debug --prefix=$(CURRENT_DIR)/install CPPFLAGS="-DRTM -I$(TSX_ROOT)/lib/rtm" LDFLAGS="-L$(TSX_ROOT)/lib/rtm -lrtm"

bench: bdb
	cd bench && make

all-clean:
	cd bench && make clean
	cd build_unix && make distclean
	rm -rf install

