.PHONY: origin clean all-clean

PARSECDIR=${TSX_ROOT}/benchmark/parsec-3.0

origin:  clean
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netstreamcluster

clean:
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a clean -c gcc-pthreads -p netstreamcluster
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a uninstall -c gcc-pthreads -p netstreamcluster

all-clean: clean

