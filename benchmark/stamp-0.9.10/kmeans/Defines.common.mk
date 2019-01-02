# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


hostname := $(shell hostname)

CFLAGS += -DUSE_TLH
CFLAGS += -DOUTPUT_TO_STDOUT

ifeq ($(enable_IBM_optimizations),yes)
CFLAGS += -DALIGNED_ALLOC_MEMORY
#CFLAGS += -DLARGE_ALIGNMENT
endif

PROG := kmeans

SRCS += \
	cluster.c \
	common.c \
	kmeans.c \
	normal.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

RUNPARAMSLOW := -m40 -n40 -t0.00001 -i inputs/random-n65536-d32-c16.txt
RUNPARAMSHIGH := -m15 -n15 -t0.00001 -i inputs/random-n65536-d32-c16.txt

.PHONY:	runlow1 runlow2 runlow4 runlow6 runlow8 runlow12 runlow16 runlow32 runlow64 runlow128 runlow-16 runlow-32 runlow-64

runlow1:
	$(PROGRAM) $(RUNPARAMSLOW) -p1

runlow2:
	$(PROGRAM) $(RUNPARAMSLOW) -p2

runlow4:
	$(PROGRAM) $(RUNPARAMSLOW) -p4

runlow6:
	$(PROGRAM) $(RUNPARAMSLOW) -p6

runlow8:
	$(PROGRAM) $(RUNPARAMSLOW) -p8

runlow12:
	$(PROGRAM) $(RUNPARAMSLOW) -p12

runlow16:
	$(PROGRAM) $(RUNPARAMSLOW) -p16

runlow32:
	$(PROGRAM) $(RUNPARAMSLOW) -p32

runlow64:
	$(PROGRAM) $(RUNPARAMSLOW) -p64

runlow128:
	$(PROGRAM) $(RUNPARAMSLOW) -p128

runlow-16:
	$(PROGRAM) $(RUNPARAMSLOW) -p-16

runlow-32:
	$(PROGRAM) $(RUNPARAMSLOW) -p-32

runlow-64:
	$(PROGRAM) $(RUNPARAMSLOW) -p-64


.PHONY:	runhigh1 runhigh2 runhigh4 runhigh6 runhigh8 runhigh12 runhigh16 runhigh32 runhigh64 runhigh128 runhigh-16 runhigh-32 runhigh-64

runhigh1:
	$(PROGRAM) $(RUNPARAMSHIGH) -p1

runhigh2:
	$(PROGRAM) $(RUNPARAMSHIGH) -p2

runhigh4:
	$(PROGRAM) $(RUNPARAMSHIGH) -p4

runhigh6:
	$(PROGRAM) $(RUNPARAMSHIGH) -p6

runhigh8:
	$(PROGRAM) $(RUNPARAMSHIGH) -p8

runhigh12:
	$(PROGRAM) $(RUNPARAMSHIGH) -p12

runhigh16:
	$(PROGRAM) $(RUNPARAMSHIGH) -p16

runhigh32:
	$(PROGRAM) $(RUNPARAMSHIGH) -p32

runhigh64:
	$(PROGRAM) $(RUNPARAMSHIGH) -p64

runhigh128:
	$(PROGRAM) $(RUNPARAMSHIGH) -p128

runhigh-16:
	$(PROGRAM) $(RUNPARAMSHIGH) -p-16

runhigh-32:
	$(PROGRAM) $(RUNPARAMSHIGH) -p-32

runhigh-64:
	$(PROGRAM) $(RUNPARAMSHIGH) -p-64

# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
