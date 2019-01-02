.PHONY: all clean origin rtm
define make_instance
	rm -rf *.o histo;
	@echo $(CLASS);
	$(MAKE) -f $(2) $(3);
	rm -rf build_$(1);
	mkdir -p build_$(1);
	mv pbzip2 build_$(1);
endef

all: origin rtm

origin: Makefile.all
	$(call make_instance,$@,$^)
rtm: Makefile.all
	$(call make_instance,$@,$^,USE_RTM=YES)

clean:
	make -f Makefile.all clean
	rm -rf build_* *.o
