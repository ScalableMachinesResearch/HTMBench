include Makefile.var

TARGET = SSCA2-opt

OBJS = SSCA2.o init.o utils.o genScalData.o gen2DTorus.o \
    computeGraph.o getStartLists.o findSubGraphs.o  \
    betweennessCentrality.opt.o

CFLAGS += -DTX_SPLIT

.c.o: defs.h Makefile
	$(CC) $(INC) $(CFLAGS) -c $<


all: sprng $(OBJS) defs.h Makefile
	$(CC) $(INC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIB)

sprng: 
	(cd sprng2.0; $(MAKE); cd ..)

cleansprng: 
	(cd sprng2.0; $(MAKE) clean; cd ..)

clean: cleansprng
	rm -f *.o *~ $(TARGET) core*
