
.PHONY: origin opt clean 

origin: clean
	-$(MAKE) -j -f Makefile.origin.spec
opt: clean
	-$(MAKE) -j -f Makefile.opt.spec
clean:
	-$(MAKE) -j -f Makefile.origin.spec clean
	-$(MAKE) -j -f Makefile.opt.spec clean
