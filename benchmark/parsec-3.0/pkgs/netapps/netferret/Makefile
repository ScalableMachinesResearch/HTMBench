.PHONY: origin opt clean all-clean 

PARSECDIR=${TSX_ROOT}/benchmark/parsec-3.0

origin: inputs clean
	-cp src/server/Makefile.origin src/server/Makefile
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netferret

opt: inputs clean
	-cp src/server/Makefile.opt src/server/Makefile
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netferret

inputs:
	ln -s ${TSX_INPUT}/parsec-3.0/pkgs/apps/ferret/inputs inputs

clean:
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a clean -c gcc-pthreads -p netferret
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a uninstall -c gcc-pthreads -p netferret

all-clean: clean
	-rm -f *.dat.ddp src/server/Makefile
	-unlink inputs
