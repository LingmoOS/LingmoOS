# Because variables can be masked at anypoint by declaring
# PASS_VAR, we need to call all variables as $(call xx,VAR)
# This little bit of magic makes it possible:
xx=$(if $($(curpass)_$(1)),$($(curpass)_$(1)),$($(1)))
define generic_multilib_extra_pkg_install
set -e; \
mkdir -p debian/$(1)/usr/include/sys; \
ln -sf $(DEB_HOST_MULTIARCH)/bits debian/$(1)/usr/include/; \
ln -sf $(DEB_HOST_MULTIARCH)/gnu debian/$(1)/usr/include/; \
ln -sf $(DEB_HOST_MULTIARCH)/fpu_control.h debian/$(1)/usr/include/; \
for i in `ls debian/tmp/usr/include/$(DEB_HOST_MULTIARCH)/sys`; do \
	ln -sf ../$(DEB_HOST_MULTIARCH)/sys/$$i debian/$(1)/usr/include/sys/$$i; \
done
mkdir -p debian/$(1)/usr/include/finclude; \
for i in `ls debian/tmp/usr/include/finclude/$(DEB_HOST_MULTIARCH)`; do \
	ln -sf $(DEB_HOST_MULTIARCH)/$$i debian/$(1)/usr/include/finclude/$$i; \
done
endef

ifneq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
    libc_extra_config_options = $(extra_config_options) --disable-sanity-checks \
                               --enable-hacker-mode
endif

ifdef WITH_SYSROOT
    libc_extra_config_options += --with-headers=$(WITH_SYSROOT)/$(includedir)
endif

$(stamp)config_sub_guess: $(stamp)patch
	@echo Updating config.sub and config.guess
	dh_update_autotools_config
	touch $@

$(patsubst %,mkbuilddir_%,$(GLIBC_PASSES)) :: mkbuilddir_% : $(stamp)mkbuilddir_%
$(stamp)mkbuilddir_%:
	@echo Making builddir for $(curpass)
	test -d $(DEB_BUILDDIR) || mkdir -p $(DEB_BUILDDIR)
	touch $@

$(patsubst %,configure_%,$(GLIBC_PASSES)) :: configure_% : $(stamp)configure_%
$(stamp)configure_%: $(stamp)config_sub_guess $(stamp)patch $(KERNEL_HEADER_DIR) $(stamp)mkbuilddir_%
	@echo Configuring $(curpass)
	rm -f $(DEB_BUILDDIR)/configparms
	echo "MIG = $(call xx,MIG)"               >> $(DEB_BUILDDIR)/configparms
	echo "BUILD_CC = $(BUILD_CC)"             >> $(DEB_BUILDDIR)/configparms
	echo "BUILD_CXX = $(BUILD_CXX)"           >> $(DEB_BUILDDIR)/configparms
	echo "CFLAGS = $(HOST_CFLAGS)"            >> $(DEB_BUILDDIR)/configparms
	echo "ASFLAGS = $(HOST_CFLAGS)"           >> $(DEB_BUILDDIR)/configparms
	echo "BUILD_CFLAGS = $(BUILD_CFLAGS)"     >> $(DEB_BUILDDIR)/configparms
	echo "LDFLAGS = "                         >> $(DEB_BUILDDIR)/configparms
	echo "BASH := /bin/bash"                  >> $(DEB_BUILDDIR)/configparms
	echo "KSH := /bin/bash"                   >> $(DEB_BUILDDIR)/configparms
	echo "SHELL := /bin/bash"                 >> $(DEB_BUILDDIR)/configparms
ifeq (,$(filter stage1 stage2,$(DEB_BUILD_PROFILES)))
	if [ "$(curpass)" = "libc" ]; then \
	  echo "LIBGD = yes"                      >> $(DEB_BUILDDIR)/configparms; \
	else \
	  echo "LIBGD = no"                       >> $(DEB_BUILDDIR)/configparms; \
	fi
else
	echo "LIBGD = no"                         >> $(DEB_BUILDDIR)/configparms
endif
	echo "bindir = $(bindir)"                 >> $(DEB_BUILDDIR)/configparms
	echo "datadir = $(datadir)"               >> $(DEB_BUILDDIR)/configparms
	echo "complocaledir = $(complocaledir)"   >> $(DEB_BUILDDIR)/configparms
	echo "sysconfdir = $(sysconfdir)"         >> $(DEB_BUILDDIR)/configparms
	echo "libexecdir = $(libexecdir)"         >> $(DEB_BUILDDIR)/configparms
	echo "rootsbindir = $(rootsbindir)"       >> $(DEB_BUILDDIR)/configparms
	echo "includedir = $(call xx,includedir)" >> $(DEB_BUILDDIR)/configparms
	echo "docdir = $(docdir)"                 >> $(DEB_BUILDDIR)/configparms
	echo "mandir = $(mandir)"                 >> $(DEB_BUILDDIR)/configparms
	echo "sbindir = $(sbindir)"               >> $(DEB_BUILDDIR)/configparms
	echo "vardbdir = $(vardbdir)"             >> $(DEB_BUILDDIR)/configparms
	echo "libdir = $(call xx,libdir)"         >> $(DEB_BUILDDIR)/configparms
	echo "slibdir = $(call xx,slibdir)"       >> $(DEB_BUILDDIR)/configparms
	echo "rtlddir = $(call xx,rtlddir)"       >> $(DEB_BUILDDIR)/configparms

	# Define the installation directory for all calls to make. This avoid
	# broken glibc makefiles to spuriously trigger install rules trying to
	# overwrite system headers.
	echo "install_root = $(CURDIR)/$(debian-tmp)" >> $(DEB_BUILDDIR)/configparms

	# Per architecture debian specific tests whitelist
	echo "include $(CURDIR)/debian/testsuite-xfail-debian.mk" >> $(DEB_BUILDDIR)/configparms

	# Prevent autoconf from running unexpectedly by setting it to false.
	# Also explicitly pass CC down - this is needed to get -m64 on
	# Sparc, et cetera.
	configure_build=$(call xx,configure_build); \
	if [ $(call xx,configure_target) = $$configure_build ]; then \
	  echo "Checking that we're running at least kernel version: $(call xx,MIN_KERNEL_SUPPORTED)"; \
	  if ! $(call kernel_check,$(call xx,MIN_KERNEL_SUPPORTED)); then \
	    configure_build=`echo $$configure_build | sed 's/^\([^-]*\)-\([^-]*\)$$/\1-dummy-\2/'`; \
	    echo "No.  Forcing cross-compile by setting build to $$configure_build."; \
	  fi; \
	fi; \
	echo -n "Build started: " ; date --rfc-2822; \
	echo "---------------"; \
	cd $(DEB_BUILDDIR) && \
		CC="$(call xx,CC) -U_FILE_OFFSET_BITS -U_TIME_BITS" \
		CXX=$(if $(filter nocheck,$(DEB_BUILD_OPTIONS)),:,"$(call xx,CXX) -U_FILE_OFFSET_BITS -U_TIME_BITS") \
		MIG="$(call xx,MIG)" \
		AUTOCONF=false \
		MAKEINFO=: \
		$(CURDIR)/configure \
		--host=$(call xx,configure_target) \
		--build=$$configure_build --prefix=/usr \
		--enable-add-ons=$(standard-add-ons)"$(call xx,add-ons)" \
		--without-selinux \
		--disable-crypt \
		--enable-bind-now \
		--enable-fortify-source \
		--enable-stackguard-randomization \
		--enable-stack-protector=strong \
		--with-pkgversion="Debian GLIBC $(DEB_VERSION)" \
		--with-bugurl="http://www.debian.org/Bugs/" \
		--with-timeoutfactor="$(TIMEOUTFACTOR)" \
		$(if $(filter $(pt_chown),yes),--enable-pt_chown) \
		$(if $(filter $(threads),no),--disable-nscd) \
		$(if $(filter $(call xx,mvec),no),--disable-mathvec) \
		$(if $(filter -Wno-error,$(shell dpkg-buildflags --get CFLAGS)),--disable-werror) \
		$(call xx,with_headers) $(call xx,extra_config_options)
	touch $@

$(patsubst %,build_%,$(GLIBC_PASSES)) :: build_% : $(stamp)build_%
$(stamp)build_%: $(stamp)configure_%
	@echo Building $(curpass)

ifneq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
	$(MAKE) cross-compiling=yes -C $(DEB_BUILDDIR) $(NJOBS) csu/subdir_lib
else
	$(MAKE) -C $(DEB_BUILDDIR) $(NJOBS)
	echo "---------------"
	echo -n "Build ended: " ; date --rfc-2822
endif
	touch $@

$(patsubst %,check_%,$(GLIBC_PASSES)) :: check_% : $(stamp)check_%
$(stamp)check_%: $(stamp)build_%
	@set -e ; \
	if [ -n "$(filter nocheck,$(DEB_BUILD_OPTIONS))" ]; then \
	  echo "Tests have been disabled via DEB_BUILD_OPTIONS." ; \
	elif [ $(call xx,configure_build) != $(call xx,configure_target) ] && \
	     ! $(DEB_BUILDDIR)/elf/ld.so $(DEB_BUILDDIR)/libc.so >/dev/null 2>&1 ; then \
	  echo "Flavour cross-compiled, tests have been skipped." ; \
	elif echo $(DEB_HOST_ARCH_CPU) | grep -q mips && \
	     $(call xx,CC) -o $(DEB_BUILDDIR)/testsuite-mips-nan2008 debian/testsuite-mips-nan2008.c && \
	     ! $(DEB_BUILDDIR)/testsuite-mips-nan2008 ; then \
	  echo "CPU NaN encoding does not match the ABI, tests have been skipped" ; \
	elif ! $(call kernel_check,$(call xx,MIN_KERNEL_SUPPORTED)); then \
	  echo "Kernel too old, tests have been skipped." ; \
	elif [ $(call xx,RUN_TESTSUITE) != "yes" ]; then \
	  echo "Testsuite disabled for $(curpass), skipping tests."; \
	else \
	  find $(DEB_BUILDDIR) -name '*.out' -delete ; \
	  LD_PRELOAD="" LANG="" LANGUAGE="" $(MAKE) -C $(DEB_BUILDDIR) $(NJOBS) check || true ; \
	  if ! test -f $(DEB_BUILDDIR)/tests.sum ; then \
	    echo "+---------------------------------------------------------------------+" ; \
	    echo "|                     Testsuite failed to build.                      |" ; \
	    echo "+---------------------------------------------------------------------+" ; \
	    exit 1 ; \
	  fi ; \
	  echo "+---------------------------------------------------------------------+" ; \
	  echo "|                             Testsuite summary                       |" ; \
	  echo "+---------------------------------------------------------------------+" ; \
	  grep -E '^(FAIL|XFAIL|XPASS):' $(DEB_BUILDDIR)/tests.sum | sort ; \
	  for test in $$(sed -e '/^\(FAIL\|XFAIL\): /!d;s/^.*: //' $(DEB_BUILDDIR)/tests.sum) ; do \
	    echo "----------" ; \
	    cat $(DEB_BUILDDIR)/$$test.test-result ; \
	    if test -f $(DEB_BUILDDIR)/$$test.out ; then \
	      cat $(DEB_BUILDDIR)/$$test.out ; \
	    fi ; \
	    echo "----------" ; \
	  done ; \
	  if grep -q '^FAIL:' $(DEB_BUILDDIR)/tests.sum ; then \
	    echo "+---------------------------------------------------------------------+" ; \
	    echo "|     Encountered regressions that don't match expected failures.     |" ; \
	    echo "+---------------------------------------------------------------------+" ; \
	    grep -E '^FAIL:' $(DEB_BUILDDIR)/tests.sum | sort ; \
	    if ! echo $(DEB_VERSION) | grep -q -E '^Version:.*\+deb[0-9]+u[0-9]+' ; then \
	        grep -E '^FAIL:' $(DEB_BUILDDIR)/tests.sum | sort > $@_failed ; \
	    fi ; \
	  else \
	    echo "+---------------------------------------------------------------------+" ; \
	    echo "| Passed regression testing.  Give yourself a hearty pat on the back. |" ; \
	    echo "+---------------------------------------------------------------------+" ; \
	    touch $@_passed ; \
	  fi ; \
	fi
	touch $@

build-arch-post-check: $(patsubst %,$(stamp)check_%,$(GLIBC_PASSES))
	@echo "CHECK SUMMARY"
	@for pass in $(patsubst %,$(stamp)check_%,$(GLIBC_PASSES)); do \
	  if [ -f $${pass}_passed ]; then \
	    echo "check for $$(basename $$pass) passed"; \
	  fi; \
	done
	@fail=0; \
	for pass in $(patsubst %,$(stamp)check_%,$(GLIBC_PASSES)); do \
	  if [ -f $${pass}_failed ]; then \
	    echo "check for $$(basename $$pass) failed"; \
	    sed -e 's/^/  /' $${pass}_failed ; \
	    fail=1; \
	  fi; \
	done; \
	exit $$fail

# Make sure to use the just built iconvconfig for native builds. When
# cross-compiling use the system iconvconfig. A cross-specific
# build-dependency makes sure that the correct version is used, as
# the format might change between upstream versions.
ifeq ($(DEB_BUILD_ARCH),$(DEB_HOST_ARCH))
ICONVCONFIG = $(CURDIR)/$(DEB_BUILDDIRLIBC)/elf/ld.so --library-path $(CURDIR)/$(DEB_BUILDDIRLIBC):$(CURDIR)/$(DEB_BUILDDIRLIBC)/mach:$(CURDIR)/$(DEB_BUILDDIRLIBC)/hurd \
	      $(CURDIR)/$(DEB_BUILDDIRLIBC)/iconv/iconvconfig
else
ICONVCONFIG = /usr/sbin/iconvconfig
endif

$(patsubst %,install_%,$(GLIBC_PASSES)) :: install_% : $(stamp)install_%
$(stamp)install_%: $(stamp)build_%
	@echo Installing $(curpass)
	rm -rf $(CURDIR)/$(debian-tmp)
ifneq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
	$(MAKE) -C $(DEB_BUILDDIR) $(NJOBS) \
	    cross-compiling=yes install_root=$(CURDIR)/$(debian-tmp)	\
	    install-bootstrap-headers=yes install-headers

	install -d $(CURDIR)/$(debian-tmp)/$(call xx,libdir)
	install -m 644 $(DEB_BUILDDIR)/csu/crt[01in].o $(CURDIR)/$(debian-tmp)/$(call xx,libdir)/.
	$(call xx,CC) -nostdlib -nostartfiles -shared -x c /dev/null \
	        -o $(CURDIR)/$(debian-tmp)/$(call xx,libdir)/libc.so
else
	$(MAKE) -C $(DEB_BUILDDIR) \
	  install_root=$(CURDIR)/$(debian-tmp) install

	# Generate gconv-modules.cache
	case $(curpass)-$(call xx,slibdir) in libc-* | *-/lib32 | *-/lib64 | *-/libo32 | *-/libx32) \
	  $(ICONVCONFIG) --nostdlib --prefix=$(CURDIR)/$(debian-tmp) \
			 -o $(CURDIR)/$(debian-tmp)/$(call xx,libdir)/gconv/gconv-modules.cache \
			 $(call xx,libdir)/gconv \
	  ;; \
	esac

	# Generate the list of SUPPORTED locales
	if [ $(curpass) = libc ]; then \
	  $(MAKE) -f debian/generate-supported.mk IN=localedata/SUPPORTED \
	    OUT=$(debian-tmp)/usr/share/i18n/SUPPORTED; \
	fi

ifeq ($(DEB_HOST_ARCH_OS),linux)
	# Install the Python pretty printers
	mkdir -p $(CURDIR)/$(debian-tmp)/usr/share/gdb/auto-load/$(call xx,slibdir)
	perl -pe 'BEGIN {undef $$/; open(IN, "$(DEB_BUILDDIR)/nptl/nptl_lock_constants.py"); $$j=<IN>;} s/from nptl_lock_constants import \*/$$j/g;' \
		$(CURDIR)/nptl/nptl-printers.py > $(CURDIR)/$(debian-tmp)/usr/share/gdb/auto-load/$(call xx,slibdir)/$(libc_so)-gdb.py
endif

ifeq ($(DEB_HOST_ARCH_OS),linux)
	# Install an empty libpthread_nonshared.a to support broken closed
	# source software.
	ar crv $(CURDIR)/$(debian-tmp)/$(call xx,libdir)/libpthread_nonshared.a
endif

	# Newly added upstream architectures do not get an empty libanl.a
	# installed, ensure there is always one to avoid porting issues.
	ar crv $(CURDIR)/$(debian-tmp)/$(call xx,libdir)/libanl.a
endif

	# Create the multiarch directories, and the configuration file in /etc/ld.so.conf.d
	if [ $(curpass) = libc ]; then \
	  mkdir -p $(debian-tmp)/etc/ld.so.conf.d; \
	  conffile="$(debian-tmp)/etc/ld.so.conf.d/$(DEB_HOST_MULTIARCH).conf"; \
	  echo "# Multiarch support" > $$conffile; \
	  echo "/usr/local/lib/$(DEB_HOST_MULTIARCH)" >> $$conffile; \
	  echo "$(call xx,slibdir)" >> $$conffile; \
	  echo "$(call xx,libdir)" >> $$conffile; \
	  if [ "$(DEB_HOST_GNU_TYPE)" != "$(DEB_HOST_MULTIARCH)" ]; then \
	    echo "/usr/local/lib/$(DEB_HOST_GNU_TYPE)" >> $$conffile; \
	    echo "/lib/$(DEB_HOST_GNU_TYPE)" >> $$conffile; \
	    echo "/usr/lib/$(DEB_HOST_GNU_TYPE)" >> $$conffile; \
	  fi; \
	  mkdir -p $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/bits $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/gnu $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/sys $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/fpu_control.h $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/a.out.h $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/ieee754.h $(debian-tmp)/usr/include/$(DEB_HOST_MULTIARCH); \
	  mkdir -p $(debian-tmp)/usr/include/finclude/$(DEB_HOST_MULTIARCH); \
	  mv $(debian-tmp)/usr/include/finclude/math-vector-fortran.h $(debian-tmp)/usr/include/finclude/$(DEB_HOST_MULTIARCH); \
	fi

ifeq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
	# For our biarch libc, add an ld.so.conf.d configuration; this
	# is needed because multiarch libc Replaces: libc6-i386 for ld.so, and
	# the multiarch ld.so doesn't look at the (non-standard) /lib32, so we
	# need path compatibility when biarch and multiarch packages are both
	# installed.
	case $(call xx,slibdir) in /lib32 | /lib64 | /libo32 | /libx32) \
	  mkdir -p $(debian-tmp)/etc/ld.so.conf.d; \
	  conffile="$(debian-tmp)/etc/ld.so.conf.d/zz_$(curpass)-biarch-compat.conf"; \
	  echo "# Legacy biarch compatibility support" > $$conffile; \
	  echo "$(call xx,slibdir)" >> $$conffile; \
	  echo "$(call xx,libdir)" >> $$conffile; \
	  ;; \
	esac

	# Move the dynamic linker into the slibdir location and replace it with
	# a symlink. This is needed:
	# - for TCC which is not able to find the dynamic linker if it is not
	#   in a lib directory.
	# - for co-installation for multiarch and biarch libraries
	# In case slibdir and rtlddir are the same directory (for instance on
	# libc6-amd64:i386), we instead rename the dynamic linker to ld.so
	rtld_so=$(rtld_so) ; \
	rtlddir=$(call xx,rtlddir) ; \
	slibdir=$(call xx,slibdir) ; \
	if [ "$$rtlddir" = "$$slibdir" ] ; then \
	  mv $(debian-tmp)$$slibdir/$$rtld_so $(debian-tmp)$$slibdir/ld.so ; \
	  ln -s $$slibdir/ld.so $(debian-tmp)$$slibdir/$$rtld_so ; \
	else \
	  mv $(debian-tmp)$$rtlddir/$$rtld_so $(debian-tmp)$$slibdir ; \
	  ln -s $$slibdir/$$rtld_so $(debian-tmp)$$rtlddir/$$rtld_so ; \
	fi

	$(call xx,extra_install)
endif

	# With Rules-Requires-Root=no, the upstream makefile fails to set the
	# correct chmod for pt_chown as it tries to set the owner at the same
	# time. Fix the permissions, dpkg-deb will "fix" the owner.
ifeq ($(pt_chown),yes)
	if [ $$(stat -c "%u" $(CURDIR)/$(debian-tmp)/usr/lib/pt_chown) != 0 ]; then \
	  chmod 4755 $(CURDIR)/$(debian-tmp)/usr/lib/pt_chown ; \
	fi
endif

	# Create /var/cache/ldconfig
	install -m 700 -d $(CURDIR)/$(debian-tmp)/var/cache/ldconfig

	touch $@

#
# Make sure to use the just built localedef for native builds. When
# cross-compiling use the system localedef passing --little-endian
# or --big-endian to select the correct endianess. A cross-specific
# build-dependency makes sure that the correct version is used, as
# the format might change between upstream versions.
#
ifeq ($(DEB_BUILD_ARCH),$(DEB_HOST_ARCH))
LOCALEDEF = I18NPATH=$(CURDIR)/localedata \
	    GCONV_PATH=$(CURDIR)/$(DEB_BUILDDIRLIBC)/iconvdata \
	    LC_ALL=C \
	    $(CURDIR)/$(DEB_BUILDDIRLIBC)/elf/ld.so --library-path $(CURDIR)/$(DEB_BUILDDIRLIBC):$(CURDIR)/$(DEB_BUILDDIRLIBC)/mach:$(CURDIR)/$(DEB_BUILDDIRLIBC)/hurd \
	    $(CURDIR)/$(DEB_BUILDDIRLIBC)/locale/localedef
else
LOCALEDEF = I18NPATH=$(CURDIR)/localedata \
	    GCONV_PATH=$(CURDIR)/$(DEB_BUILDDIRLIBC)/iconvdata \
	    LC_ALL=C \
	    localedef --$(DEB_HOST_ARCH_ENDIAN)-endian
endif

$(stamp)build_C.utf8: $(stamp)/build_libc
	$(LOCALEDEF) --quiet -c -f UTF-8 -i C $(CURDIR)/build-tree/C.utf8
	touch $@

$(stamp)build_locales-all: $(stamp)/build_libc
	$(MAKE) -C $(DEB_BUILDDIRLIBC) $(NJOBS) \
		objdir=$(DEB_BUILDDIRLIBC) \
		install_root=$(CURDIR)/build-tree/locales-all \
		localedata/install-locale-files LOCALEDEF="$(LOCALEDEF)"
	# Remove the C.utf8 locale to avoid conflicts with the one in libc-bin
	rm -fr $(CURDIR)/build-tree/locales-all/usr/lib/locale/C.utf8
	rdfind -outputname /dev/null -makesymlinks true -removeidentinode false \
		$(CURDIR)/build-tree/locales-all/usr/lib/locale
	symlinks -r -s -c $(CURDIR)/build-tree/locales-all/usr/lib/locale
	touch $@

$(stamp)source: $(stamp)patch
	mkdir -p $(build-tree)
	find $(GLIBC_SOURCES) -print0 | \
	LC_ALL=C sort -z | \
	tar -c -J --null --no-recursion -T - \
		--mode=go=rX,u+rw,a-s \
		--clamp-mtime --mtime "@$(SOURCE_DATE_EPOCH)" \
		--owner=root --group=root --numeric-owner \
		--xform='s=^=glibc-$(DEB_VERSION_UPSTREAM)/=' \
		-f $(CURDIR)/$(build-tree)/glibc-$(DEB_VERSION_UPSTREAM).tar.xz
	mkdir -p debian/glibc-source/usr/src/glibc
	tar cf - --files-from debian/glibc-source.filelist \
		--clamp-mtime --mtime "@$(SOURCE_DATE_EPOCH)" \
		| tar -x -C debian/glibc-source/usr/src/glibc -f -

	touch $@

.NOTPARALLEL: $(patsubst %,check_%,$(GLIBC_PASSES))
