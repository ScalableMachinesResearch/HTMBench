CLASS ?= C
UPDATE ?= au

.PHONY: all clean origin rtm
all: origin rtm

origin: Makefile.origin
	rm -rf *.o ua
	@echo $(CLASS)
	$(MAKE) -f Makefile.$@ CLASS=${CLASS} UPDATE=$(UPDATE)
	rm -rf build_$@
	mkdir -p build_$@
	mv ua build_$@/
rtm: Makefile.rtm
	rm -rf *.o ua
	@echo $(CLASS)
	$(MAKE) -f Makefile.$@ CLASS=${CLASS} UPDATE=$(UPDATE)
	rm -rf build_$@
	mkdir -p build_$@
	mv ua build_$@/

.PHONY: clean
clean:
	make -f Makefile.origin clean
	make -f Makefile.rtm clean
	rm -rf build_origin build_rtm *.o
