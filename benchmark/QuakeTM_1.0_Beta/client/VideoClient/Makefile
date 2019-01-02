#
# QuakeWorld Makefile for Linux 2.0
#
# Apr '98 by Zoid <zoid@idsoftware.com>
#
# GNU Make required
#
# ELF only
#

# RPM release number
RPM_RELEASE=1

VERSION=2.30$(GLIBC)

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6* ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif


MAINDIR=$(shell pwd)

BUILD_DEBUG_DIR=debug
#$(GLIBC)

CC=gcc
DEBUG_CFLAGS=-Wall -g -DGLQUAKE -Dstricmp=strcasecmp -I$(MAINDIR)

LDFLAGS=-lm
GL_X11_LDFLAGS=-DGLQUAKE -L/usr/X11/lib  -lm -lGL -lX11 -lXext

DO_GL_CC=$(CC) $(CFLAGS) $(GLCFLAGS) -o $@ -c $<
DO_GL_AS=$(CC) $(CFLAGS) $(GLCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################


TARGETS= $(BUILDDIR)/glqwclient

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/object_files
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"


all:	build_debug 

targets: $(TARGETS)




#############################################################################
# GL CLIENT
#############################################################################

GLQWCL_OBJS = \
	$(BUILDDIR)/object_files/cl_demo.o \
	$(BUILDDIR)/object_files/cl_ents.o \
	$(BUILDDIR)/object_files/cl_input.o \
	$(BUILDDIR)/object_files/cl_main.o \
	$(BUILDDIR)/object_files/cl_parse.o \
	$(BUILDDIR)/object_files/cl_pred.o \
	$(BUILDDIR)/object_files/cl_tent.o \
	$(BUILDDIR)/object_files/cl_cam.o \
	$(BUILDDIR)/object_files/cmd.o \
	$(BUILDDIR)/object_files/common.o \
	$(BUILDDIR)/object_files/console.o \
	$(BUILDDIR)/object_files/crc.o \
	$(BUILDDIR)/object_files/cvar.o \
	$(BUILDDIR)/object_files/keys.o \
	$(BUILDDIR)/object_files/mathlib.o \
	$(BUILDDIR)/object_files/md4.o \
	$(BUILDDIR)/object_files/menu.o \
	$(BUILDDIR)/object_files/net_chan.o \
	$(BUILDDIR)/object_files/net_udp.o \
	$(BUILDDIR)/object_files/nonintel.o \
	$(BUILDDIR)/object_files/pmove.o \
	$(BUILDDIR)/object_files/pmovetst.o \
	$(BUILDDIR)/object_files/r_part.o \
	$(BUILDDIR)/object_files/sbar.o \
	$(BUILDDIR)/object_files/skin.o \
	$(BUILDDIR)/object_files/snd_dma.o \
	$(BUILDDIR)/object_files/snd_mem.o \
	$(BUILDDIR)/object_files/snd_mix.o \
	$(BUILDDIR)/object_files/view.o \
	$(BUILDDIR)/object_files/wad.o \
	$(BUILDDIR)/object_files/zone.o \
	$(BUILDDIR)/object_files/cd_linux.o \
	$(BUILDDIR)/object_files/sys_linux.o \
	$(BUILDDIR)/object_files/snd_linux.o \
	\
	$(BUILDDIR)/object_files/gl_draw.o \
	$(BUILDDIR)/object_files/gl_mesh.o \
	$(BUILDDIR)/object_files/gl_model.o \
	$(BUILDDIR)/object_files/gl_ngraph.o \
	$(BUILDDIR)/object_files/gl_refrag.o \
	$(BUILDDIR)/object_files/gl_rlight.o \
	$(BUILDDIR)/object_files/gl_rmain.o \
	$(BUILDDIR)/object_files/gl_rmisc.o \
	$(BUILDDIR)/object_files/gl_rsurf.o \
	$(BUILDDIR)/object_files/gl_screen.o \
	$(BUILDDIR)/object_files/gl_warp.o \
	$(BUILDDIR)/object_files/gl_vidlinuxglx.o
#	\
#	$(BUILDDIR)/object_files/math.o \
#	$(BUILDDIR)/object_files/snd_mixa.o \
#	$(BUILDDIR)/object_files/sys_dosa.o \



$(BUILDDIR)/glqwclient : $(GLQWCL_OBJS) 
	$(CC) $(CFLAGS) -o $@ $(GLQWCL_OBJS) $(LDFLAGS) $(GL_X11_LDFLAGS)

$(BUILDDIR)/object_files/cl_demo.o :       cl_demo.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_ents.o :       cl_ents.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_input.o :      cl_input.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_main.o :       cl_main.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_parse.o :      cl_parse.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_pred.o :       cl_pred.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_tent.o :       cl_tent.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cl_cam.o :        cl_cam.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cmd.o :           cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/common.o :        common.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/console.o :       console.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/crc.o :           crc.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cvar.o :          cvar.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/keys.o :          keys.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/mathlib.o :       mathlib.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/md4.o :           md4.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/menu.o :          menu.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/net_chan.o :      net_chan.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/net_udp.o :       net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/nonintel.o :      nonintel.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/pmove.o :         pmove.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/pmovetst.o :      pmovetst.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/r_part.o :        r_part.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/sbar.o :          sbar.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/screen.o :        screen.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/skin.o :          skin.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/snd_dma.o :       snd_dma.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/snd_mem.o :       snd_mem.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/snd_mix.o :       snd_mix.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/view.o :          view.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/wad.o :           wad.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/zone.o :          zone.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/cd_linux.o :      cd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/sys_linux.o :     sys_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/snd_linux.o :     snd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_draw.o :       gl_draw.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_mesh.o :       gl_mesh.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_model.o :      gl_model.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_ngraph.o :     gl_ngraph.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_refrag.o :     gl_refrag.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_rlight.o :     gl_rlight.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_rmain.o :      gl_rmain.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_rmisc.o :      gl_rmisc.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_rsurf.o :      gl_rsurf.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_screen.o :     gl_screen.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_vidlinuxglx.o :   gl_vidlinuxglx.c
	$(DO_GL_CC)

$(BUILDDIR)/object_files/gl_warp.o :       gl_warp.c
	$(DO_GL_CC)

#$(BUILDDIR)/object_files/math.o :          math.s
#	$(DO_GL_AS)

#$(BUILDDIR)/object_files/snd_mixa.o :      snd_mixa.s
#	$(DO_GL_AS)

#$(BUILDDIR)/object_files/sys_dosa.o :      sys_dosa.s
#	$(DO_GL_AS)



#############################################################################
# MISC
#############################################################################

clean:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:	
	-rm -f $(GLQWCL_OBJS) 
	-rm -R $(BUILD_DEBUG_DIR)

