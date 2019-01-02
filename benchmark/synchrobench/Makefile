.PHONY:	clean all

all:  bin/RTM-linkedlist bin/RTM-linkedlist-locknest bin/RTM-skiplist

bin/RTM-linkedlist:
	$(MAKE) "STM=RTM" -C src/linkedlists/lockfree-list origin

bin/RTM-linkedlist-locknest:
	$(MAKE) "STM=RTM" -C src/linkedlists/lockfree-list locknest

bin/RTM-skiplist:
	$(MAKE) "STM=RTM" -C src/skiplists/sequential

clean:
	rm -rf build
	$(MAKE) -C src/utils/estm-0.3.0 clean
	rm -rf build bin
