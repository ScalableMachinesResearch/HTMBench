# (c) 2007 The Board of Trustees of the University of Illinois.
.PHONY: all clean origin rtm
define make_instance
	rm -rf *.o histo;
	@echo $(CLASS);
	$(MAKE) -f Makefile.$(1);
	rm -rf build_$(1);
	mkdir -p build_$(1);
	mv histo build_$(1);
endef

all: origin atomic rtm_naive rtm_txn

origin: Makefile.origin
	$(call make_instance,$@)
atomic: Makefile.atomic
	$(call make_instance,$@)
rtm_naive: Makefile.rtm_naive
	$(call make_instance,$@)
rtm_txn: Makefile.rtm_txn
	$(call make_instance,$@)

clean:
	make -f Makefile.origin clean
	make -f Makefile.atomic clean
	make -f Makefile.rtm_naive clean
	make -f Makefile.rtm_txn clean
	rm -rf build_* *.o
