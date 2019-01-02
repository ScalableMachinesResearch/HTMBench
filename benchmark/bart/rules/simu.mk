# Copyright 2013. The Regents of the University of California.
# All rights reserved. Use of this source code is governed by 
# a BSD-style license which can be found in the LICENSE file.




simusrcs := $(wildcard $(srcdir)/simu/*.c)
simuobjs := $(simusrcs:.c=.o)

.INTERMEDIATE: $(simuobjs)

lib/libsimu.a: libsimu.a($(simuobjs))


UTARGETS += test_biot_savart
MODULES_test_biot_savart += -lsimu

