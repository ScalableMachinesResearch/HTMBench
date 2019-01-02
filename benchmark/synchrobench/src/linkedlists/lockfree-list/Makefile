ROOT = ../../..

include $(ROOT)/common/Makefile.common

.PHONY:	all clean origin locknest


all: origin locknest

origin:
	$(MAKE) -f Makefile.origin

locknest:
	$(MAKE) -f Makefile.locknest

clean:
	$(MAKE) -f Makefile.origin clean
	$(MAKE) -f Makefile.locknest clean

