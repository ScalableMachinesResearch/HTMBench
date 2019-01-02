
TM_FLAGS=-g -fno-inline -O0 #-DHTM_STATS

OBJS= $(patsubst %.c,%.o,$(wildcard *.c))

CC = gcc

.PHONY: clean rtm rtm_rle

all: rtm rtm_rle

rtm:
	rm -f *.o
	$(CC) $(CFLAGS) $(TM_FLAGS) -fpic -c *.c -lm
	$(CC) -shared -o librtm.so $(OBJS)  -lm

rtm_rle:
	rm -rf *.o
	$(CC) $(CFLAGS) $(TM_FLAGS) -DGLOCK_RW -fpic -c *.c -lm
	$(CC) -shared -o librtm_rle.so $(OBJS)  -lm
	if [ ! -f librtm_rw.so ]; then\
		ln -s librtm_rle.so librtm_rw.so;\
	fi

#librtm.so : $(OBJS)
#	$(CC) -shared -o $@ $^ -lm

#%.o: %.c
#	$(CC) $(CFLAGS) $(TM_FLAGS) -fpic -c -o $@ $< -lm

clean:
	rm -f *.o *.so
