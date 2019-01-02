#
# QuakeWorld Makefile for Linux 2.0
#
# Apr '98 by Zoid <zoid@idsoftware.com>
#
# GNU Make required
#
# ELF only
#

# Changes to original file made by Vladimir Gajinov



# get the current working directory
MAINDIR=$(shell pwd)
BUILD_DEBUG_DIR=$(MAINDIR)
SERVER_DIR=$(MAINDIR)

# define the compiler vendor
COMPILER=GCC

#Intel STM compiler prototype version - 3.0
ifeq ($(COMPILER), INTEL)
	CC=icc	
	CFLAGS=-openmp -Wall -O3 -I$(SERVER_DIR)
	CFLAGS+= -diag-disable 111,593,810,869,981,1418,1419,1572,1286,1287,2259 
	DEFS :=-DINTEL
endif

	CFLAGS=-Qtm_enabled -openmp -Wall -O3 -I$(SERVER_DIR) -diag-disable 111,593,810,869,981,1418,1419,1572,1286,1287,2259 

ifeq ($(COMPILER), GCC)
#gcc
CC=gcc
CFLAGS=-fopenmp  -Wall -O3 -fno-inline -Dstricmp=strcasecmp -I$(SERVER_DIR) -g
CFLAGS += -I${TSX_ROOT}/lib/rtm
CFLAGS+= -Wno-unused-variable

#When the 32-bit Intel STM compiler is run on a 64-bit system libraries have to be linked statically
#LDFLAGS=-static -Bdynamic /usr/lib/libc.so -lm -L/usr/lib32 
LDFLAGS=-lm
LDFLAGS += -L${TSX_ROOT}/lib/rtm -lrtm
DO_SERVER_CC=$(CC) -DSERVERONLY $(CFLAGS) -o $@ -c $<


endif

#############################################################################
# PARALLEL STAGE & DEBUG OUTPUT
#############################################################################

# define which frame stage to do in parallel
# can be READ_PACKETS, PHYSICS and/or SEND_REPLY
# example: DEFS+= -DDOPARALLEL=READ_PACKETS+PHYSICS
#DEFS+= -DDOPARALLEL=READ_PACKETS
DEFS+= -DDOPARALLEL=READ_PACKETS+PHYSICS+SEND_REPLY

# define the debug info output by the server
# define DEBUG macro in form of DEBUG_REGION + DEBUG_LEVEL
# DEBUG_REGION = (READ_PACKETS, PHYSICS, SEND_REPLY and/or GENERAL)
# DEBUG_LEVEL =  (NONE, LOW, MEDIUM or HIGH)
# example DEFS+= -DDEBUG=READ_PACKETS+PHYSICS+MEDIUM
DEFS+= -DDEBUG=GENERAL+MEDIUM

#############################################################################
# SPECIFIC TARGETS
#############################################################################
run=tm_coarse
ifeq ($(run), global)
	DEFS+= -DDO_GLOBAL -DDO_TM -DDO_NESTED
else ifeq ($(run), lock_coarse)
	DEFS+= -DDO_LOCK -DDO_COARSE
else ifeq ($(run), lock_fine)
	DEFS+= -DDO_LOCK -DDO_FINE
else ifeq ($(run), tm_coarse)
	DEFS+= -DDO_TM -DDO_COARSE
else ifeq ($(run), tm_fine)
	DEFS+= -DDO_TM -DDO_FINE
else ifeq ($(run), tm_fine_nested)
	DEFS+= -DDO_TM -DDO_FINE -DDO_NESTED
else ifeq  ($(run), lock_tm)
	DEFS+= -DDO_LOCK -DDO_TM -DDO_COARSE
else ifeq ($(run), serial)
	DEFS :=-DINTEL -DDOPARALLEL=0 -DDEBUG=GENERAL+MEDIUM
else
	# run tm_fine as default
	DEFS := $(DEFS) -DDO_TM -DDO_FINE
endif
CFLAGS+= $(DEFS)


#############################################################################
# SETUP AND BUILD
#############################################################################


TARGETS=$(BUILDDIR)/qwsv 


all: 
	echo Defined macros: $(DEFS)
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR)

targets: $(TARGETS)

#############################################################################
# SERVER
#############################################################################


QWSV_OBJS := $(patsubst %.c,$(BUILDDIR)/%.o,$(wildcard *.c))

$(BUILDDIR)/qwsv : $(QWSV_OBJS)
	$(CC) $(CFLAGS) -o $@ $(QWSV_OBJS) $(LDFLAGS)

# we cannot use this because some order of compilation has to be respected
#%.o :  %.c
#   $(CC) -DSERVERONLY $(CFLAGS) $^ -o $@



$(BUILDDIR)/tm_lib.o :   $(SERVER_DIR)/tm_lib.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/tm_vsprintf.o :   $(SERVER_DIR)/tm_vsprintf.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_cmds.o :   $(SERVER_DIR)/pr_cmds.c 
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_edict.o :  $(SERVER_DIR)/pr_edict.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pr_exec.o :   $(SERVER_DIR)/pr_exec.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_init.o :   $(SERVER_DIR)/sv_init.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_main.o :   $(SERVER_DIR)/sv_main.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_nchan.o :  $(SERVER_DIR)/sv_nchan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_ents.o :   $(SERVER_DIR)/sv_ents.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_send.o :   $(SERVER_DIR)/sv_send.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_move.o :   $(SERVER_DIR)/sv_move.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_phys.o :   $(SERVER_DIR)/sv_phys.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_user.o :   $(SERVER_DIR)/sv_user.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sv_ccmds.o :  $(SERVER_DIR)/sv_ccmds.c
	$(DO_SERVER_CC)

$(BUILDDIR)/world.o :     $(SERVER_DIR)/world.c
	$(DO_SERVER_CC)

$(BUILDDIR)/sys_unix.o :  $(SERVER_DIR)/sys_unix.c
	$(DO_SERVER_CC)

$(BUILDDIR)/model.o :     $(SERVER_DIR)/model.c
	$(DO_SERVER_CC)

$(BUILDDIR)/cmd.o :       $(SERVER_DIR)/cmd.c
	$(DO_SERVER_CC)

$(BUILDDIR)/common.o :    $(SERVER_DIR)/common.c
	$(DO_SERVER_CC)

$(BUILDDIR)/crc.o :       $(SERVER_DIR)/crc.c
	$(DO_SERVER_CC)

$(BUILDDIR)/cvar.o :      $(SERVER_DIR)/cvar.c
	$(DO_SERVER_CC)

$(BUILDDIR)/mathlib.o :   $(SERVER_DIR)/mathlib.c
	$(DO_SERVER_CC)

$(BUILDDIR)/md4.o :       $(SERVER_DIR)/md4.c
	$(DO_SERVER_CC)

$(BUILDDIR)/zone.o :      $(SERVER_DIR)/zone.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pmove.o :     $(SERVER_DIR)/pmove.c
	$(DO_SERVER_CC)

$(BUILDDIR)/pmovetst.o :  $(SERVER_DIR)/pmovetst.c
	$(DO_SERVER_CC)

$(BUILDDIR)/net_chan.o :  $(SERVER_DIR)/net_chan.c
	$(DO_SERVER_CC)

$(BUILDDIR)/net_udp.o :   $(SERVER_DIR)/net_udp.c
	$(DO_SERVER_CC)

$(BUILDDIR)/tm_io.o : 		 $(SERVER_DIR)/tm_io.c
	$(DO_SERVER_CC)


#############################################################################
# MISC
#############################################################################

.PHONY: clean

clean: 
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR)

clean2:
	-rm -f $(QWSV_OBJS)
	-rm -f qwsv


