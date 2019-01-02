.PHONY: all clean
SUBDIRS := ScalParC UtilityMine

all: $(SUBDIRS)
	for d in $^ ; do \
		$(MAKE) -C $$d ; \
	done;

clean: $(SUBDIRS)
	for d in $^ ; do \
		$(MAKE) -C $$d clean ; \
	done;
	
