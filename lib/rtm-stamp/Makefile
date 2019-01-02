
TM_FLAGS=-g -O3 -fno-inline -DUSE_TLH -DLIST_NO_DUPLICATES -DCHUNK_STEP1=2 -DHTM_IBM 

OBJS= thread.o htm_util.o htm_ibm.o

ifdef FORTRAN_FLAG
ifneq ($(FORTRAN_FLAG), "")
	TM_FLAGS += -DFORTRAN_FLAG
endif
endif

CC = gcc

all: clean librtm.so

librtm.so : $(OBJS)
	$(CC) -shared -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(TM_FLAGS) -fpic -c -o $@ $<

clean:
	rm -f *.o librtm.so
