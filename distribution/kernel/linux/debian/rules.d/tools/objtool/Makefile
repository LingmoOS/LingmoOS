PROGS = \
	objtool \
	objtool.real-powerpc \
	objtool.real-x86

include $(top_rulesdir)/Makefile.inc

objtool.real-%:
	mkdir -p $*
# objtool always uses HOSTCC, HOSTLD, and HOSTAR; we need to override
# these on the command line to make cross-builds work.  But it also
# builds fixdep which still needs to be native in a cross-build.  Set
# REALHOSTCC and REALHOSTLD variables which will be used for fixdep.
	$(MAKE) -C $(top_srcdir)/tools/objtool O=$(CURDIR)/$* \
		HOSTARCH=$(KERNEL_ARCH) ARCH=$* \
		HOSTCC=$(CC) KBUILD_HOSTCFLAGS='$(CFLAGS) $(CPPFLAGS)' \
		HOSTLD=$(CROSS_COMPILE)ld KBUILD_HOSTLDFLAGS='$(LDFLAGS)' \
		HOSTAR=$(CROSS_COMPILE)ar \
		REALHOSTCC=gcc REALHOSTLD=ld \
		V=1
	ln -f $*/objtool $@

%: %.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
