# Should each of these have per-package options?

$(patsubst %,binaryinst_%,$(DEB_ARCH_REGULAR_PACKAGES) $(DEB_INDEP_REGULAR_PACKAGES)) :: binaryinst_% : $(stamp)binaryinst_%

# Make sure the debug packages are built last, since other packages may add
# files to them.
debug-packages = $(filter %-dbg,$(DEB_ARCH_REGULAR_PACKAGES))
non-debug-packages = $(filter-out %-dbg,$(DEB_ARCH_REGULAR_PACKAGES))
$(patsubst %,$(stamp)binaryinst_%,$(debug-packages)):: $(patsubst %,$(stamp)binaryinst_%,$(non-debug-packages))

ifeq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
DH_STRIP_DEBUG_PACKAGE=--dbg-package=$(libc)-dbg
endif

$(patsubst %,$(stamp)binaryinst_%,$(DEB_ARCH_REGULAR_PACKAGES) $(DEB_INDEP_REGULAR_PACKAGES)):: $(patsubst %,$(stamp)install_%,$(GLIBC_PASSES)) debhelper
	@echo Running debhelper for $(curpass)
	dh_testroot
	dh_installdirs -p$(curpass)
	dh_install -p$(curpass)
	dh_installman -p$(curpass)
	dh_installinfo -p$(curpass)
	dh_installdebconf -p$(curpass)
	if [ $(curpass) = glibc-doc ] ; then \
		dh_installchangelogs -p$(curpass) ; \
	else \
		dh_installchangelogs -p$(curpass) debian/changelog.upstream ; \
	fi
	dh_installinit -p$(curpass)
	dh_installtmpfiles -p$(curpass)
	dh_installsystemd -p$(curpass)
	dh_installdocs -p$(curpass) 
	dh_lintian -p $(curpass)

	# Ensure that symlinks resolve even when /usr is unmerged.
	set -e; \
	find "debian/$(curpass)" \( -lname "*../lib*" -o -lname "/lib*" \) -printf "%p*%l\n" | \
	while IFS='*' read -r p l; do \
	  ln -svf "$${l%%/lib*}/usr/lib$${l#*/lib}" "$$p"; \
	done

	dh_link -p$(curpass)
	dh_bugfiles -p$(curpass)

	# when you want to install extra packages, use extra_pkg_install.
	$(call xx,extra_pkg_install)

ifeq ($(filter nostrip,$(DEB_BUILD_OPTIONS)),)
	if test "$(NOSTRIP_$(curpass))" != 1; then					\
	  if test "$(DEBUG_$(curpass))" = 1; then					\
	    dh_strip -p$(curpass) $(DH_STRIP_DEBUG_PACKAGE);				\
	  else										\
	    dh_strip -p$(curpass);							\
	  fi ;										\
	  for f in $$(find debian/$(curpass) -name \*crt\*.o) ; do			\
	    $(DEB_HOST_GNU_TYPE)-strip --strip-debug --remove-section=.comment		\
	                               --remove-section=.note $$f ;			\
	  done ;									\
	fi
endif

	dh_compress -p$(curpass)
	# Keep the setuid on pt_chown (non-Linux only).
	# Keep the 0700 permissions of /var/cache/ldconfig
	dh_fixperms -p$(curpass) -Xpt_chown -Xvar/cache/ldconfig
	# libc.so prints useful version information when executed.
	find debian/$(curpass) -type f -regex '.*/libc\.so\.[0-9.]+' -exec chmod a+x '{}' ';'
	# Use this instead of -X to dh_fixperms so that we can use
	# an unescaped regular expression.  ld.so must be executable;
	find debian/$(curpass) -type f -name 'ld.so' -exec chmod a+x '{}' ';'
	find debian/$(curpass) -type f -regex '.*/ld.*\.so\.[0-9]' -exec chmod a+x '{}' ';'
	dh_makeshlibs -Xgconv/ -p$(curpass) -V "$(call xx,shlib_dep)"
	# Add relevant udeb: lines in shlibs files
	sh ./debian/shlibs-add-udebs $(curpass)

	dh_installdeb -p$(curpass)
	dh_shlibdeps -p$(curpass) $(foreach path,$($(lastword $(subst -, ,$(curpass)))_slibdir),-l/usr$(path))
	dh_gencontrol -p$(curpass)
	dh_md5sums -p$(curpass)

	# We adjust the compression format depending on the package:
	# - we slightly increase the compression level for locales-all as it
	#   contains highly compressible data
	# - other packages use dpkg's default xz format
	case $(curpass) in \
	locales-all ) \
		dh_builddeb -p$(curpass) -- -Zxz -z7 ;; \
	*) \
		dh_builddeb -p$(curpass) ;; \
	esac

	touch $@

$(patsubst %,binaryinst_%,$(DEB_UDEB_PACKAGES)) :: binaryinst_% : $(stamp)binaryinst_%
$(patsubst %,$(stamp)binaryinst_%,$(DEB_UDEB_PACKAGES)): debhelper $(patsubst %,$(stamp)install_%,$(GLIBC_PASSES))
	@echo Running debhelper for $(curpass)
	dh_testroot
	dh_installdirs -p$(curpass)
	dh_install -p$(curpass)
	dh_strip -p$(curpass)

	# Ensure that symlinks resolve even when /usr is unmerged.
	set -e; \
	find "debian/$(curpass)" \( -lname "*../lib*" -o -lname "/lib*" \) -printf "%p*%l\n" | \
	while IFS='*' read -r p l; do \
	  ln -svf "$${l%%/lib*}/usr/lib$${l#*/lib}" "$$p"; \
	done

	dh_link -p$(curpass)
	
	# when you want to install extra packages, use extra_pkg_install.
	$(call xx,extra_pkg_install)

	dh_compress -p$(curpass)
	dh_fixperms -p$(curpass)
	find debian/$(curpass) -type f -regex '.*/libc\.so\.[0-9.]+' -exec chmod a+x '{}' ';'
	find debian/$(curpass) -type f -regex '.*/ld.*\.so\.[0-9]' -exec chmod a+x '{}' ';'
	dh_installdeb -p$(curpass)
	# dh_shlibdeps -p$(curpass)
	dh_gencontrol -p$(curpass)
	dh_builddeb -p$(curpass)

	touch $@

debhelper: $(stamp)debhelper-common $(patsubst %,$(stamp)debhelper_%,$(GLIBC_PASSES))
$(stamp)debhelper-common: 
	for x in `find debian/debhelper.in -maxdepth 1 -type f`; do \
	  y=debian/`basename $$x`; \
	  perl -p \
	      -e 'BEGIN {local $$/=undef; open(IN, "debian/script.in/nsscheck.sh"); $$j=<IN>;} s/__NSS_CHECK__/$$j/g;' \
	      -e 'BEGIN {open(IN, "debian/tmp/usr/share/i18n/SUPPORTED"); $$l = join("", grep { !/^C\.UTF-8/ } grep { /UTF-8/ } <IN>);} s/__PROVIDED_LOCALES__/$$l/g;' \
	      -e 's#DEB_VERSION_UPSTREAM#$(DEB_VERSION_UPSTREAM)#g;' \
	      -e 's#CURRENT_VER#$(DEB_VERSION)#g;' \
	      $$x > $$y ; \
	  case $$y in \
	    *.install) \
	      $(if $(filter $(pt_chown),no),sed -e "/pt_chown/d" -i $$y ;) \
	      $(if $(filter $(pldd),no),sed -e "/pldd/d" -i $$y ;) \
	      ;; \
	  esac; \
	done

	# Install nscd systemd files on linux
ifeq ($(DEB_HOST_ARCH_OS),linux)
	cp nscd/nscd.service debian/nscd.service
	cp nscd/nscd.tmpfiles debian/nscd.tmpfiles
endif

	# Generate common substvars files.
	: > tmp.substvars
ifeq ($(filter stage1 stage2,$(DEB_BUILD_PROFILES)),)
	echo 'libgcc:Depends=libgcc-s1 [!hppa !m68k], libgcc-s2 [m68k], libgcc-s4 [hppa]' >> tmp.substvars
	echo 'libcrypt-dev:Depends=libcrypt-dev' >> tmp.substvars
	echo 'rpcsvc-proto:Depends=rpcsvc-proto' >> tmp.substvars
	echo 'libc-dev:Breaks=$(libc)-dev-$(DEB_HOST_ARCH)-cross (<< $(DEB_VERSION_UPSTREAM)~)' >> tmp.substvars
endif
	for pkg in $(DEB_ARCH_REGULAR_PACKAGES) $(DEB_INDEP_REGULAR_PACKAGES) $(DEB_UDEB_PACKAGES); do \
	  cp tmp.substvars debian/$$pkg.substvars; \
	done
	rm -f tmp.substvars

	touch $@

ifneq ($(filter stage1,$(DEB_BUILD_PROFILES)),)
$(patsubst %,debhelper_%,$(GLIBC_PASSES)) :: debhelper_% : $(stamp)debhelper_%
$(stamp)debhelper_%: $(stamp)debhelper-common $(stamp)install_%
	libdir=$(call xx,libdir) ; \
	slibdir=$(call xx,slibdir) ; \
	rtlddir=$(call xx,rtlddir) ; \
	curpass=$(curpass) ; \
	rtld_so=$(rtld_so) ; \
	rtld_target=$(rtld_target) ; \
	templates="libc-dev" ;\
	pass="" ; \
	suffix="" ;\
	case "$$curpass:$$slibdir" in \
	  libc:*) \
	    ;; \
	  *:/lib32 | *:/lib64 | *:/libo32 | *:/libx32) \
	    pass="-alt" \
	    suffix="-$(curpass)" \
	    ;; \
	  *:* ) \
           templates="" \
	    ;; \
	esac ; \
	for t in $$templates ; do \
	  for s in debian/$$t$$pass.* ; do \
	    t=`echo $$s | sed -e "s#libc\(.*\)$$pass#$(libc)\1$$suffix#"` ; \
	    echo "Generating $$t ..."; \
	    if [ "$$s" != "$$t" ] ; then \
	      cp $$s $$t ; \
	    fi ; \
	    sed -i \
		-e "/usr\/lib\/.*\.a/d" \
		-e "/LIBDIR.*\.a /d" \
		-e "s#TMPDIR#$(debian-tmp)#g" \
		-e "s#RTLDDIR#$$rtlddir#g" \
		-e "s#SLIBDIR#$$slibdir#g" \
		-e "s#LIBDIR#$$libdir#g" \
	        -e "s#RTLD_SO#$$rtld_so#g" \
	        -e "s#RTLD_TARGET#$$rtld_target#g" \
		-e "/gdb/d" \
		-e "/audit/d" \
	      $$t; \
	  done ; \
	done
else
$(patsubst %,debhelper_%,$(GLIBC_PASSES)) :: debhelper_% : $(stamp)debhelper_%
$(stamp)debhelper_%: $(stamp)debhelper-common $(stamp)install_%
	libdir=$(call xx,libdir) ; \
	slibdir=$(call xx,slibdir) ; \
	rtlddir=$(call xx,rtlddir) ; \
	curpass=$(curpass) ; \
	rtld_so=$(rtld_so) ; \
	rtld_target=$(rtld_target) ; \
	case "$$curpass:$$slibdir" in \
	  libc:*) \
	    templates="libc libc-dev libc-udeb" \
	    pass="" \
	    suffix="" \
	    ;; \
	  *:/lib32 | *:/lib64 | *:/libo32 | *:/libx32) \
	    templates="libc libc-dev" \
	    pass="-alt" \
	    suffix="-$(curpass)" \
	    ;; \
	esac ; \
	for t in $$templates ; do \
	  for s in debian/$$t$$pass.* ; do \
	    t=`echo $$s | sed -e "s#libc\(.*\)$$pass#$(libc)\1$$suffix#"` ; \
	    if [ "$$s" != "$$t" ] ; then \
	      cp $$s $$t ; \
	    fi ; \
	    sed -e "s#TMPDIR#$(debian-tmp)#g" -i $$t; \
	    sed -e "s#RTLDDIR#$$rtlddir#g" -i $$t; \
	    sed -e "s#SLIBDIR#$$slibdir#g" -i $$t; \
	    sed -e "s#LIBDIR#$$libdir#g" -i $$t; \
	    sed -e "s#RTLD_SO#$$rtld_so#g" -i $$t ; \
	    sed -e "s#RTLD_TARGET#$$rtld_target#g" -i $$t ; \
	    $(if $(filter $(call xx,mvec),no),sed -e "/libmvec/d" \
	                                          -e "/libm-\*\.a/d" \
	                                          -e "/lacks-unversioned-link-to-shared-library.*libm\.so/d" \
	                                          -e "/unpack-message-for-deb-data.*libm\.a/d" \
	                                          -i $$t ;) \
	    $(if $(filter-out $(DEB_HOST_ARCH_OS),linux),sed -e "/gdb/d" -i $$t ;) \
	  done ; \
	done
endif

	touch $@

clean::
	dh_clean 

	rm -f debian/*.install
	rm -f debian/*.install.*
	rm -f debian/*.manpages
	rm -f debian/*.links
	rm -f debian/*.postinst
	rm -f debian/*.preinst
	rm -f debian/*.postinst
	rm -f debian/*.prerm
	rm -f debian/*.postrm
	rm -f debian/*.init
	rm -f debian/*.config
	rm -f debian/*.templates
	rm -f debian/*.dirs
	rm -f debian/*.docs
	rm -f debian/*.lintian-overrides
	rm -f debian/*.NEWS
	rm -f debian/*.README.Debian
	rm -f debian/*.triggers
	rm -f debian/*.service
	rm -f debian/*.tmpfiles

	rm -f $(stamp)binaryinst*
