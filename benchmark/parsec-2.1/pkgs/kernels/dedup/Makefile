.PHONY: all clean origin rtm
define make_instance
	rm -rf *.o dedup;
	$(MAKE) -f Makefile.$(1);
	rm -rf build_$(1);
	mkdir -p build_$(1);
	mv dedup build_$(1);
endef

all: origin rtm

origin: Makefile.origin
	$(call make_instance,origin)
rtm: Makefile.rtm
	$(call make_instance,rtm)

clean:
	make -f Makefile.origin clean
	make -f Makefile.rtm clean
	rm -rf build_origin build_rtm *.o
