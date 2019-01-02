

SUBDIR= netdedup netferret  netstreamcluster 


.PHONY: all-clean clean

all-clean:
	for d in $(SUBDIR); do \
		$(MAKE) -C $$d all-clean;\
	done

clean:
	for d in $(SUBDIR); do \
		$(MAKE) -C $$d clean;\
	done
