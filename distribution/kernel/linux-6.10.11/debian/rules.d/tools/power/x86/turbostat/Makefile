PROGS = turbostat

installdir = /usr/sbin

include $(top_rulesdir)/Makefile.inc

CPPFLAGS += -I"$(top_srcdir)/tools/include" -DMSRHEADER='"$(top_srcdir)/arch/x86/include/asm/msr-index.h"' -DINTEL_FAMILY_HEADER='"$(top_srcdir)/arch/x86/include/asm/intel-family.h"' -DBUILD_BUG_HEADER='"$(top_srcdir)/include/linux/build_bug.h"'

LDLIBS += -lcap -lrt
