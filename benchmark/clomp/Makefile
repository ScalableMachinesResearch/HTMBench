# Makefile for CLOMP_TM
# Please see COPYRIGHT AND LICENSE at bottom of Makefile

TM_DIR=${TSX_ROOT}/lib/rtm
CFLAGS = -DDIVIDE_CALC -DCALC_STRIDE=4 -g -fopenmp -O3 -fno-inline --openmp --param inline-unit-growth=200


.PHONY: all origin rtm_small rtm_large clean
all: origin rtm_small rtm_large

origin: clomp_tm.c
	@echo "=> Starting compile of clomp_tm without TM for gcc (-DNOTM)"
	gcc -DDIVIDE_CALC -DCALC_STRIDE=4 --openmp -O3  --param inline-unit-growth=200 -DNOTM clomp_tm.c -o clomp -lm
	rm -rf build_origin
	mkdir -p build_origin
	mv clomp build_origin/
	@echo "=> Compile of clomp_tm with gcc and without TM completed."
	@echo "\n*** MUST SET setenv GOMP_SPINCOUNT 100000 TO GET GOOD TIMINGS for gcc"

rtm_small: clomp_tm.c clomp_tm.h
	@echo "=> Starting compile of clomp_tm with RTM for gcc (-DRTM)"
	gcc $(CFLAGS) -I$(TM_DIR) -DRTM -DSMALL_TM clomp_tm.c -o clomp -lm -lpthread  -L$(TM_DIR) -lrtm
	rm -rf build_$@
	mkdir -p build_$@
	mv clomp build_$@/
	@echo "=> Compile of clomp_tm with gcc and with RTM completed."
	@echo "\n*** MUST SET setenv GOMP_SPINCOUNT 100000 TO GET GOOD TIMINGS for gcc"
rtm_large: clomp_tm.c clomp_tm.h
	@echo "=> Starting compile of clomp_tm with RTM for gcc (-DRTM)"
	gcc $(CFLAGS) -I$(TM_DIR) -DRTM -DLARGE_TM clomp_tm.c -o clomp -lm -lpthread  -L$(TM_DIR) -lrtm
	rm -rf build_$@
	mkdir -p build_$@
	mv clomp build_$@/
	@echo "=> Compile of clomp_tm with gcc and with RTM completed."
	@echo "\n*** MUST SET setenv GOMP_SPINCOUNT 100000 TO GET GOOD TIMINGS for gcc"
.PHYONY: clean
clean:
	rm -rf build_origin build_rtm*
##############################################################################
# COPYRIGHT AND LICENSE
#
#
# Copyright (c) 2012, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory
# Written by John Gyllenhaal (gyllen@llnl.gov)
# LLNL-CODE-582532
# OCEC-12-058
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# o  Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the disclaimer below.
#
# o  Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the disclaimer (as noted below) in the
#    documentation and/or other materials provided with the distribution.
#
# o  Neither the name of the LLNS/LLNL nor the names of its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL
# SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Additional BSD Notice
#
#  1. This notice is required to be provided under our contract with the
#     U.S. Department of Energy (DOE). This work was produced at
#     Lawrence Livermore National Laboratory under Contract
#     No. DE-AC52-07NA27344 with the DOE.
#
#  2. Neither the United States Government nor Lawrence Livermore National
#     Security, LLC nor any of their employees, makes any warranty, express
#     or implied, or assumes any liability or responsibility for the accuracy,
#     completeness, or usefulness of any information, apparatus, product, or
#     process disclosed, or represents that its use would not infringe
#     privately-owned rights.
#
# 3. Also, reference herein to any specific commercial products, process, or
#    services by trade name, trademark, manufacturer or otherwise does not
#    necessarily constitute or imply its endorsement, recommendation, or
#    favoring by the United States Government or Lawrence Livermore National
#    Security, LLC. The views and opinions of authors expressed herein do not
#    necessarily state or reflect those of the United States Government or
#    Lawrence Livermore National Security, LLC, and shall not be used for
#    advertising or product endorsement purposes.
##############################################################################
