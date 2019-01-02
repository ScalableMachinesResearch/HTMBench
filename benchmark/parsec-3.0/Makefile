

SUBDIR= netapps  


.PHONY: all-clean clean 

all-clean:
	for d in $(SUBDIR); do \
		$(MAKE) -C pkgs/$$d all-clean;\
	done
	rm -rf log	

clean:
	for d in $(SUBDIR); do \
		$(MAKE) -C pkgs/$$d clean;\
	done

	
