# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


hostname := $(shell hostname)

PROG := intruder

SRCS += \
	decoder.c \
	detector.c \
	dictionary.c \
	intruder.c \
	packet.c \
	preprocessor.c \
	stream.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/rbtree.c \
	$(LIB)/hashtable.c \
	$(LIB)/conc_hashtable.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DUSE_TLH

ifeq ($(enable_IBM_optimizations),yes)
CFLAGS += -DMAP_USE_CONCUREENT_HASHTABLE -DHASHTABLE_SIZE_FIELD -DHASHTABLE_RESIZABLE
CFLAGS += -DUSE_RBTREE_FOR_FRAGMENT_REASSEMBLE -DRBTREE_SIZE_FIELD
else
CFLAGS += -DMAP_USE_RBTREE
endif

RUNPARAMS := -a10 -l128 -n262144 -s1

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
