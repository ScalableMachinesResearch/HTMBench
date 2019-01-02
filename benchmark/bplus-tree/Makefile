# Configurable options
#   MODE = release | debug (default: debug)
#   SNAPPY = 0 | 1 (default: 1)
#

MODE=release
SNAPPY=0

RTM_LIB=$(TSX_ROOT)/lib/rtm

CSTDFLAG = --std=c99 -pedantic -Wall -Wextra -Wno-unused-parameter
CPPFLAGS += -fPIC -Iinclude -Iexternal/snappy -I$(RTM_LIB)
CPPFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DRTM
CPPFLAGS += -D_XOPEN_SOURCE=500 -D_DARWIN_C_SOURCE
LDFLAGS += -Wl,-rpath=$(RTM_LIB) -lpthread -L$(RTM_LIB) -lrtm



ifeq ($(MODE),release)
	CPPFLAGS += -O3
	DEFINES += -DNDEBUG
	CFLAGS += -g -fno-inline
else
	CFLAGS += -g
endif

# run make with SNAPPY=0 to turn it off
ifneq ($(SNAPPY),0)
	DEFINES += -DBP_USE_SNAPPY=1
	TARGETS = external/snappy/config.status bplus.a
else
	DEFINES += -DBP_USE_SNAPPY=0
	TARGETS =  bplus.a
endif


TESTS =
TESTS += test-threaded-rw

OBJS =
OBJS += src/utils.o
OBJS += src/writer.o
OBJS += src/values.o
OBJS += src/pages.o
OBJS += src/bplus.o


ifneq ($(SNAPPY),0)
	OBJS += external/snappy/snappy-sinksource.o
	OBJS += external/snappy/snappy.o
	OBJS += external/snappy/snappy-c.o
endif

deps := $(OBJS:%.o=%.o.d)


all: $(TESTS) 

external/snappy/config.status:
	(cd external/snappy && ./autogen.sh && ./configure)

bplus.a: $(OBJS)
	$(AR) rcs bplus.a $(OBJS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(CSTDFLAG) $(CPPFLAGS) $(DEFINES) \
		-o $@ -MMD -MF $@.d -c $<

external/snappy/%.o: external/snappy/%.cc
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%: test/%.cc bplus.a
	$(CXX) $(CFLAGS) $(CPPFLAGS) $< -o $@ bplus.a $(LDFLAGS)

clean:
	@rm -f bplus.a
	@rm -f $(OBJS) $(TESTS) $(deps)

.PHONY: all check clean

-include $(deps)
