CC=gcc
CFLAGS= -O3 -g -I../install/include -I${TSX_ROOT}/lib/rtm -DRTM
LDFLAGS= -Wl,-rpath=../install/lib -L../install/lib -ldb -lpthread -L${TSX_ROOT}/lib/rtm -lrtm

.PHONY: clean outputclean

ex_thread: ex_thread.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

outputclean:
	-rm -f __db.* log.00* access.db
clean: outputclean
	-rm -f ex_thread
