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
BUILD_DEBUG_DIR= $(MAINDIR)

CC=gcc
DEBUG_CFLAGS=-Wall -g -o0 -Dstricmp=strcasecmp -I$(MAINDIR)

LDFLAGS=-lm

DO_GL_CC=$(CC) $(CFLAGS) -o $@ -c $<


#############################################################################
# SETUP AND BUILD
#############################################################################


TARGETS= $(BUILDDIR)/glqwclient

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"


all:	build_debug 

targets: $(TARGETS)




#############################################################################
# GL CLIENT
#############################################################################

GLQWCL_OBJS = \
	$(BUILDDIR)/cl_demo.o \
	$(BUILDDIR)/cl_ents.o \
	$(BUILDDIR)/cl_input.o \
	$(BUILDDIR)/cl_main.o \
	$(BUILDDIR)/cl_parse.o \
	$(BUILDDIR)/cl_pred.o \
	$(BUILDDIR)/cl_tent.o \
	$(BUILDDIR)/cl_cam.o \
	$(BUILDDIR)/cmd.o \
	$(BUILDDIR)/common.o \
	$(BUILDDIR)/console.o \
	$(BUILDDIR)/crc.o \
	$(BUILDDIR)/cvar.o \
	$(BUILDDIR)/keys.o \
	$(BUILDDIR)/mathlib.o \
	$(BUILDDIR)/md4.o \
	$(BUILDDIR)/net_chan.o \
	$(BUILDDIR)/net_udp.o \
	$(BUILDDIR)/nonintel.o \
	$(BUILDDIR)/pmove.o \
	$(BUILDDIR)/pmovetst.o \
	$(BUILDDIR)/skin.o \
	$(BUILDDIR)/view.o \
	$(BUILDDIR)/wad.o \
	$(BUILDDIR)/zone.o \
	$(BUILDDIR)/sys_linux.o \
 	$(BUILDDIR)/gl_model.o
 		

#	$(BUILDDIR)/snd_linux.o \
#	$(BUILDDIR)/cd_linux.o \
#	$(BUILDDIR)/snd_mem.o \
#	$(BUILDDIR)/snd_mix.o \
#	$(BUILDDIR)/snd_dma.o \

#	$(BUILDDIR)/sbar.o \
#	$(BUILDDIR)/menu.o \
#	$(BUILDDIR)/r_part.o \

# 	$(BUILDDIR)/gl_draw.o \
# 	$(BUILDDIR)/gl_mesh.o \
# 	$(BUILDDIR)/gl_model.o \
# 	$(BUILDDIR)/gl_ngraph.o \
# 	$(BUILDDIR)/gl_refrag.o \
# 	$(BUILDDIR)/gl_rlight.o \
# 	$(BUILDDIR)/gl_rmain.o \
# 	$(BUILDDIR)/gl_rmisc.o \
# 	$(BUILDDIR)/gl_rsurf.o \
# 	$(BUILDDIR)/gl_screen.o \
# 	$(BUILDDIR)/gl_warp.o \
# 	$(BUILDDIR)/gl_vidlinuxglx.o
#	\
#	$(BUILDDIR)/math.o \
#	$(BUILDDIR)/snd_mixa.o \
#	$(BUILDDIR)/sys_dosa.o \



$(BUILDDIR)/glqwclient : $(GLQWCL_OBJS) 
	$(CC) $(CFLAGS) -o $@ $(GLQWCL_OBJS) $(LDFLAGS) 

$(BUILDDIR)/cl_demo.o :       cl_demo.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_ents.o :       cl_ents.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_input.o :      cl_input.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_main.o :       cl_main.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_parse.o :      cl_parse.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_pred.o :       cl_pred.c
	$(DO_GL_CC)

$(BUILDDIR)/cl_tent.o :       cl_tent.c
	$(DO_GL_CC)	

$(BUILDDIR)/cl_cam.o :        cl_cam.c
	$(DO_GL_CC)

$(BUILDDIR)/cmd.o :           cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/common.o :        common.c
	$(DO_GL_CC)

$(BUILDDIR)/console.o :       console.c
	$(DO_GL_CC)

$(BUILDDIR)/crc.o :           crc.c
	$(DO_GL_CC)

$(BUILDDIR)/cvar.o :          cvar.c
	$(DO_GL_CC)

$(BUILDDIR)/keys.o :          keys.c
	$(DO_GL_CC)

$(BUILDDIR)/mathlib.o :       mathlib.c
	$(DO_GL_CC)

$(BUILDDIR)/md4.o :           md4.c
	$(DO_GL_CC)

$(BUILDDIR)/menu.o :          menu.c
	$(DO_GL_CC)

$(BUILDDIR)/net_chan.o :      net_chan.c
	$(DO_GL_CC)

$(BUILDDIR)/net_udp.o :       net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/nonintel.o :      nonintel.c
	$(DO_GL_CC)

$(BUILDDIR)/pmove.o :         pmove.c
	$(DO_GL_CC)

$(BUILDDIR)/pmovetst.o :      pmovetst.c
	$(DO_GL_CC)

$(BUILDDIR)/sbar.o :          sbar.c
	$(DO_GL_CC)

$(BUILDDIR)/screen.o :        screen.c
	$(DO_GL_CC)

$(BUILDDIR)/skin.o :          skin.c
	$(DO_GL_CC)

$(BUILDDIR)/view.o :          view.c
	$(DO_GL_CC)

$(BUILDDIR)/wad.o :           wad.c
	$(DO_GL_CC)

$(BUILDDIR)/zone.o :          zone.c
	$(DO_GL_CC)

$(BUILDDIR)/sys_linux.o :     sys_linux.c
	$(DO_GL_CC)

 $(BUILDDIR)/gl_model.o :      gl_model.c
	$(DO_GL_CC)




#$(BUILDDIR)/snd_linux.o :     snd_linux.c
#	$(DO_GL_CC)
#$(BUILDDIR)/cd_linux.o :      cd_linux.c
#	$(DO_GL_CC)
#
#$(BUILDDIR)/snd_mem.o :       snd_mem.c
#	$(DO_GL_CC)
#
#$(BUILDDIR)/snd_mix.o :       snd_mix.c
#	$(DO_GL_CC)
#
#$(BUILDDIR)/snd_dma.o :       snd_dma.c
#	$(DO_GL_CC)




# $(BUILDDIR)/r_part.o :        r_part.c
# 	$(DO_GL_CC)


# $(BUILDDIR)/gl_draw.o :       gl_draw.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_mesh.o :       gl_mesh.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_model.o :      gl_model.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_ngraph.o :     gl_ngraph.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_refrag.o :     gl_refrag.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_rlight.o :     gl_rlight.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_rmain.o :      gl_rmain.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_rmisc.o :      gl_rmisc.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_rsurf.o :      gl_rsurf.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_screen.o :     gl_screen.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_vidlinuxglx.o :   gl_vidlinuxglx.c
# 	$(DO_GL_CC)
# 
# $(BUILDDIR)/gl_warp.o :       gl_warp.c
# 	$(DO_GL_CC)




#$(BUILDDIR)/math.o :          math.s
#	$(DO_GL_AS)

#$(BUILDDIR)/snd_mixa.o :      snd_mixa.s
#	$(DO_GL_AS)

#$(BUILDDIR)/sys_dosa.o :      sys_dosa.s
#	$(DO_GL_AS)



#############################################################################
# MISC
#############################################################################

clean:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:	
	-rm -f $(GLQWCL_OBJS)
	-rm glqwclient
	-rm -rf output 
#	-rm -R $(BUILD_DEBUG_DIR)

