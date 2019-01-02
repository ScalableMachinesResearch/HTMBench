.PHONY: build clean all-clean outputclean

PARSECDIR=${TSX_ROOT}/benchmark/parsec-3.0

build: inputs
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netdedup

inputs:
	ln -s ${TSX_INPUT}/parsec-3.0/pkgs/kernels/dedup/inputs inputs

clean:
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a clean -c gcc-pthreads -p netdedup
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a uninstall -c gcc-pthreads -p netdedup

all-clean: clean
	-rm -f *.dat.ddp
	-unlink inputs

outputclean:
	-killall -9 netdedup_client
	-killall -9 netdedup
