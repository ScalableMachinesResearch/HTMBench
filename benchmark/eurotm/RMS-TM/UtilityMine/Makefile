.PHONY: all clean all-clean

CXX  = g++ -fopenmp -g
CC = gcc -fopenmp -g
CFLAGS  = -fbuiltin -O3 -I /usr/include/c++ -I$(TSX_ROOT)/lib/rtm
OBJS	= ListItemset.o
OBJS2   = rdtsc.o TimeCalculation.o Database.o Itemset.o HashTree.o
OPTS	= -DBALT -DOPTIMAL
LIBS = -lm -L$(TSX_ROOT)/lib/rtm -lrtm

TARGET  = utility_mine

all:	$(TARGET) inputs

clean:
	rm -rf *~ *.o $(TARGET)
all-clean: clean
	unlink inputs

inputs:
	ln -s ${TSX_INPUT}/RMS-TM/utility_mine inputs

utility_mine: utility.o $(OBJS) $(OBJS2)
	$(CXX) $(CFLAGS) $(OPTS) -o utility_mine $^ $(LIBS)

utility.o : utility.cpp
	$(CXX) $(CFLAGS) $(OPTS) -c utility.cpp -o $@

rdtsc.o: rdtsc.c
	$(CC) $(CFLAGS) $(OPTS) -c $^ -o $@
#
# TimeCalculation.o: TimeCalculation.cpp
# 	$(CXX) $(CFLAGS) $(OPTS) -c TimeCalculation.cpp

Database.o: Database.cpp Database.h
	$(CXX) $(CFLAGS) $(OPTS) -c Database.cpp -o $@

Itemset.o: Itemset.cpp Itemset.h
	$(CXX) $(CFLAGS) $(OPTS) -c Itemset.cpp -o $@

ListItemset.o: ListItemset.cpp ListItemset.h
	$(CXX) $(CFLAGS) -c ListItemset.cpp -o $@

HashTree.o: HashTree.cpp HashTree.h
	$(CXX) $(CFLAGS) $(OPTS) -c HashTree.cpp -o $@
