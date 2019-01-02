
.PHONY: clean

db_bench: ../snappy/install
	-$(MAKE) -j -f Makefile.spec db_bench

clean:
	-$(MAKE) -f Makefile.spec clean

../snappy/install:
	cd ../snappy && ./autogen.sh && ./configure --prefix=$(shell pwd)/../snappy/install \
	&& make -j && make install









