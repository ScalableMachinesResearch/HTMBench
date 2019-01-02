# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


platform := $(shell uname)
architecture := $(shell uname -m)
hostname := $(shell hostname)

CFLAGS += -DUSE_TLH
CFLAGS += -DLIST_NO_DUPLICATES

ifeq ($(enable_IBM_optimizations),yes)
ifeq ($(platform),AIX)
CFLAGS += -DCHUNK_STEP1=2  # For P8
else
ifeq ($(architecture),ppc64)
CFLAGS += -DCHUNK_STEP1=2  # For P8
else
CFLAGS += -DCHUNK_STEP1=2  # Optimized default
endif
endif
else
CFLAGS += -DCHUNK_STEP1=12  # Default for original STAMP
endif

PROG := genome

SRCS += \
	gene.c \
	genome.c \
	segments.c \
	sequencer.c \
	table.c \
	$(LIB)/bitmap.c \
	$(LIB)/hash.c \
	$(LIB)/hashtable.c \
	$(LIB)/pair.c \
	$(LIB)/random.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

#RUNPARAMS := -g256 -s16 -n16384
RUNPARAMS := -g16384 -s64 -n16777216

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
