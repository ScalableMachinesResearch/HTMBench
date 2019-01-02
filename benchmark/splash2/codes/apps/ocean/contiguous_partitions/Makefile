TARGET = OCEAN
OBJS = jacobcalc.o jacobcalc2.o laplacalc.o linkup.o main.o multi.o slave1.o slave2.o subblock.o

include ../../../Makefile.config

decs.h: decs.H
jacobcalc.c: decs.h tsx.h
linkup.c: decs.h tsx.h
slave1.c: decs.h tsx.h
jacobcalc2.c: decs.h tsx.h
main.c: decs.h tsx.h tsx.h
slave2.c: decs.h tsx.h
laplacalc.c: decs.h tsx.h
multi.c: decs.h tsx.h
subblock.c: decs.h tsx.h
tsx.h: tsx.H
