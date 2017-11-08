#
# meataxe64: meataxe64
#
# This file is part of the build system of a GAP kernel extension.
# Requires automake.
#
ACLOCAL_AMFLAGS = -I m4

BINARCHDIR = bin/$(GAPARCH)
GAPINSTALLLIB = $(BINARCHDIR)/meataxe64.so

lib_LTLIBRARIES = meataxe64.la

meataxe64_la_SOURCES = \
  src/meataxe64.c \
  src/mtx64/field.c \
  src/mtx64/conex.c \
  src/mtx64/pcritGENc.c \
  src/mtx64/hpmi.c \
  src/mtx64/linf.c \
  src/mtx64/pmul.c \
  src/mtx64/slab.c

meataxe64_la_CPPFLAGS = -Isrc/mtx64 $(GAP_CPPFLAGS) -DCONFIG_H
meataxe64_la_LDFLAGS = -module -avoid-version
if SYS_IS_CYGWIN
meataxe64_la_LDFLAGS += -no-undefined -version-info 0:0:0 
meataxe64_la_LDFLAGS += -Wl,$(GAPROOT)/bin/$(GAPARCH)/gap.dll
endif

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
