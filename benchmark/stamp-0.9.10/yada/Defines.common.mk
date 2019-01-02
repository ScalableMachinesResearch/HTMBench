# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


hostname := $(shell hostname)

CFLAGS += -DUSE_TLH
CFLAGS += -DLIST_NO_DUPLICATES
CFLAGS += -DMAP_USE_AVLTREE
CFLAGS += -DSET_USE_RBTREE

PROG := yada
SRCS += \
	coordinate.c \
	element.c \
	mesh.c \
	region.c \
	yada.c \
	$(LIB)/avltree.c \
	$(LIB)/heap.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/rbtree.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

RUNPARAMS := -a15 -i inputs/ttimeu1000000.2

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
