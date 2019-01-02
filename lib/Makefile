

SUBDIR= rtm  rtm-stamp


.PHONY: all clean

all:
	for d in $(SUBDIR); do \
		$(MAKE) -C $$d;\
	done

clean:
	for d in $(SUBDIR); do \
		$(MAKE) -C $$d clean;\
	done
