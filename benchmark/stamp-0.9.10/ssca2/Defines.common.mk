# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


hostname := $(shell hostname)

PROG := ssca2

SRCS += \
	alg_radix_smp.c \
	computeGraph.c \
	createPartition.c \
	cutClusters.c \
	findSubGraphs.c \
	genScalData.c \
	getStartLists.c \
	getUserParameters.c \
	globals.c \
	ssca2.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DUSE_TLH
#CFLAGS += -DUSE_PARALLEL_DATA_GENERATION
#CFLAGS += -DWRITE_RESULT_FILES
CFLAGS += -DENABLE_KERNEL1
#CFLAGS += -DENABLE_KERNEL2 -DENABLE_KERNEL3
#CFLAGS += -DENABLE_KERNEL4

RUNPARAMS := -s20 -i1.0 -u1.0 -l3 -p3

.PHONY:	run1 run2 run4 run6 run8 run12 run16 run32 run64 run128 run-16 run-32 run-64

run1:
	$(PROGRAM) $(RUNPARAMS) -t1

run2:
	$(PROGRAM) $(RUNPARAMS) -t2

run4:
	$(PROGRAM) $(RUNPARAMS) -t4

run6:
	$(PROGRAM) $(RUNPARAMS) -t6

run8:
	$(PROGRAM) $(RUNPARAMS) -t8

run12:
	$(PROGRAM) $(RUNPARAMS) -t12

run16:
	$(PROGRAM) $(RUNPARAMS) -t16

run32:
	$(PROGRAM) $(RUNPARAMS) -t32

run64:
	$(PROGRAM) $(RUNPARAMS) -t64

run128:
	$(PROGRAM) $(RUNPARAMS) -t128

run-16:
	$(PROGRAM) $(RUNPARAMS) -t-16

run-32:
	$(PROGRAM) $(RUNPARAMS) -t-32

run-64:
	$(PROGRAM) $(RUNPARAMS) -t-64

# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
