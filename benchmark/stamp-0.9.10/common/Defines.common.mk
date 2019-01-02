# ==============================================================================
#
# Defines.common.mk
#
# ==============================================================================
# Copyright (c) IBM Corp. 2014, and others.

enable_IBM_optimizations := yes

platform := $(shell uname)
architecture := $(shell uname -m)
hostname := $(shell hostname)
ifeq ($(platform),OS/390)
CC       := c89
CFLAGS   += '-Wc,HALTON(3296),LANGLVL(extc99),FLOAT(ieee),XPLINK,ARCH(10),TUNE(10),LP64'
CFLAGS   += -D_XOPEN_SOURCE=600 -D_UNIX03_SOURCE
CFLAGS   += -O3
CFLAGS   += -I$(LIB)
CPP      := c++
CPPFLAGS += $(CFLAGS)
LD       := c89
LDFLAGS  += -Wl,XPLINK,LP64
LIBS     +=
else
ifeq ($(platform),AIX)
CC       := /usr/vac/bin/xlc_r
CFLAGS   += -O3 -q64 -g -qstrict
CFLAGS   += -I$(LIB)
CPP      := /usr/vac/bin/xlC_r
CPPFLAGS += $(CFLAGS)
LD       := /usr/vac/bin/xlc_r
LDFLAGS  += $(CFLAGS)
LIBS     += -lm
else
CC       := gcc
ifeq ($(architecture),x86_64)
CFLAGS   += -g -Wall -fno-inline -pthread  # We no longer use gcc's built-in functions for TSX.
#CFLAGS   += -g -Wall -pthread -mrtm  # For hle_intel, enable this.
else
ifeq ($(architecture),ppc64)
# We need 64-bit binaries, because bayes allocates > 3GB memory.
CFLAGS   += -g -Wall -pthread -mpowerpc64 -mcpu=power6 -m64
else
CFLAGS   += -g -Wall -pthread
endif
endif
CFLAGS   += -O3
CFLAGS   += -I$(LIB)
CPP      := g++
CPPFLAGS += $(CFLAGS)
LD       := g++
LIBS     += -lpthread
LDFLAGS  += $(CFLAGS)
endif
endif

# Remove these files when doing clean
OUTPUT +=

LIB := ../lib

STM := ../../tl2

LOSTM := ../../OpenTM/lostm


# ==============================================================================
#
# End of Defines.common.mk
#
# ==============================================================================
