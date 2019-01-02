# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.


hostname := $(shell hostname)

LIBS += -lm

PROG := labyrinth

SRCS += \
	coordinate.c \
	grid.c \
	labyrinth.c \
	maze.c \
	router.c \
	$(LIB)/list.c \
	$(LIB)/mt19937ar.c \
	$(LIB)/pair.c \
	$(LIB)/queue.c \
	$(LIB)/random.c \
	$(LIB)/thread.c \
	$(LIB)/vector.c \
	$(LIB)/memory.c
#
OBJS := ${SRCS:.c=.o}

CFLAGS += -DUSE_TLH
CFLAGS += -DUSE_EARLY_RELEASE

RUNPARAMS := -i inputs/random-x512-y512-z7-n512.txt -t

.PHONY:	run1 run2 run4 run6 run8 run12 run16 run32 run64 run128 run-16 run-32 run-64

run1:
	$(PROGRAM) $(RUNPARAMS) 1

run2:
	$(PROGRAM) $(RUNPARAMS) 2

run4:
	$(PROGRAM) $(RUNPARAMS) 4

run6:
	$(PROGRAM) $(RUNPARAMS) 6

run8:
	$(PROGRAM) $(RUNPARAMS) 8

run12:
	$(PROGRAM) $(RUNPARAMS) 12

run16:
	$(PROGRAM) $(RUNPARAMS) 16

run32:
	$(PROGRAM) $(RUNPARAMS) 32

run64:
	$(PROGRAM) $(RUNPARAMS) 64

run128:
	$(PROGRAM) $(RUNPARAMS) 128

run-16:
	$(PROGRAM) $(RUNPARAMS) -16

run-32:
	$(PROGRAM) $(RUNPARAMS) -32

run-64:
	$(PROGRAM) $(RUNPARAMS) -64

# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
