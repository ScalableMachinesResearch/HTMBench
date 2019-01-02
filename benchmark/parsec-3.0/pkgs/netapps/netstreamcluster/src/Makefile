PROGRAM = netstreamcluster

INSTDIR = ${PARSECDIR}/pkgs/netapps/netstreamcluster/inst/${PARSECPLAT}/bin

SUBDIRS = server client

.PHONY: subtargets  $(SUBDIRS) install clean 

$(PROGRAM): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

install: 
	mkdir -p ${INSTDIR}
	cp ./server/server ${INSTDIR} 
	cp ./server/server_sim ${INSTDIR} 
	cp ./client/client ${INSTDIR} 
	cp ./client/client_sim ${INSTDIR} 
	cp ./run.sh ${INSTDIR}
	#if [ ! -L ${PARSECDIR}/pkgs/netapps/netstreamcluster/inputs ]; \
        #then \
        #        ln -s ${PARSECDIR}/pkgs/kernels/streamcluster/inputs ${PARSECDIR}/pkgs/netapps/streamcluster/;\
        #fi 

clean: 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
	#unlink ${PARSECDIR}/pkgs/netapps/netstreamcluster/inputs
