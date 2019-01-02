
TM_FLAGS=-g -fno-inline -O0# -DHTM_STATS

OBJS= $(patsubst %.c,%.o,$(wildcard *.c))

CC = gcc

all: clean librtm.so

librtm.so : $(OBJS)
	$(CC) -shared -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) $(TM_FLAGS) -fpic -c -o $@ $< -lm

clean:
	rm -f *.o librtm.so
