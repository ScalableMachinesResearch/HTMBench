.PHONY: all clean all-clean
#CC = icc -openmp -g
CC = gcc -fopenmp -g
# CFLAGS = -O2 $(INCLUDE_DIR)
CFLAGS  =  -O3 -std=c99 -I$(TSX_ROOT)/lib/rtm

LINKFALGS=

LIBS = -lm -L$(TSX_ROOT)/lib/rtm -lrtm
TARGET = scalparc
OBJ  = rdtsc.o drive.o getdata.o parsort.o parclass.o

all: $(TARGET) inputs

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LINKFLAGS) $(OBJ) $(LIBS)


.c.o:
	$(CC) -c $(CFLAGS) $*.c

inputs:
	ln -s ${TSX_INPUT}/RMS-TM/ScalParC inputs

clean:
	rm -f $(TARGET) $(OBJ)
all-clean: clean
	unlink inputs
