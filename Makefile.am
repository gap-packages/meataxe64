#
# meataxe64: meataxe64
#
# This file is part of the build system of a GAP kernel extension.
# Requires automake.
#
ACLOCAL_AMFLAGS = -I m4

BINARCHDIR = bin/$(GAPARCH)
GAPINSTALLLIB = $(BINARCHDIR)/meataxe64.so

meataxe64_la_LIBADD = -lpthread

lib_LTLIBRARIES = meataxe64.la

#
# The commented out version is for OSX. You also need to make osx-pcrits.s
# using etc/asmfix.g
# TODO automate this.
#

if SYS_IS_OSX
ASM_SOURCES = src/mtx64/osx-pc1.s src/mtx64/osx-pc2.s src/mtx64/osx-pc3.s src/mtx64/osx-pc5.s   src/mtx64/osx-tfarm0.s 
else
ASM_SOURCES = src/mtx64/pc1.s src/mtx64/pc2.s src/mtx64/pc3.s src/mtx64/pc5.s  src/mtx64/tfarm0.s 
endif

meataxe64_la_SOURCES = \
   src/meataxe64.c \
   src/functions.c \
   src/slab.c \
   src/bitstring.c \
   src/mtx64/field.c \
   src/mtx64/conex.c \
   src/mtx64/pcritc.c \
   src/mtx64/hpmi.c \
   src/mtx64/linf.c \
   src/mtx64/pmul.c \
   src/mtx64/bitstring.c \
   src/mtx64/tabmake.c \
   src/mtx64/io.c \
   src/mtx64/slab.c \
   src/mtx64/dfmtf.c \
   src/mtx64/fech.c \
   src/mtx64/fmul.c \
   src/mtx64/fpef.c \
   src/mtx64/ftra.c \
   src/mtx64/funs1.c \
   src/mtx64/funs2.c \
   src/mtx64/funs3.c \
   src/mtx64/funs4.c \
   src/mtx64/funs5.c \
   src/mtx64/mmul.c \
   src/mtx64/mpef.c \
   src/mtx64/gauss.c \
   src/mtx64/proggies.c \
   src/mtx64/tfarm1.c \
   src/mtx64/tfarm2.c \
   src/mtx64/util.c \
   $(ASM_SOURCES)

meataxe64_la_CPPFLAGS = $(GAP_CPPFLAGS)
meataxe64_la_CFLAGS = $(GAP_CFLAGS)
meataxe64_la_LDFLAGS = $(GAP_LDFLAGS) -module -avoid-version
if SYS_IS_CYGWIN
meataxe64_la_LDFLAGS += -no-undefined -version-info 0:0:0 
meataxe64_la_LDFLAGS += -Wl,$(GAPROOT)/bin/$(GAPARCH)/gap.dll
endif


src/mtx64/osx-%.s : src/mtx64/%.s etc/asmfix.g
	$(GAPROOT)/bin/gap.sh -A -q etc/asmfix.g 


all-local: $(GAPINSTALLLIB)

$(GAPINSTALLLIB): meataxe64.la
	$(mkdir_p) $(BINARCHDIR)
if SYS_IS_CYGWIN
	cp .libs/meataxe64.dll $(GAPINSTALLLIB)
else
	cp .libs/meataxe64.so $(GAPINSTALLLIB)
endif

clean-local:
	rm -f $(GAPINSTALLLIB)

distclean-local:
	rm -rf bin/*

doc:
	($(GAPROOT)/bin/gap.sh -A makedoc.g)
