TESTS_DEB := \
	pkg-dir-conffile-0 pkg-dir-conffile-1 \
	pkg-dir-file-other pkg-dir-symlink-int-0 pkg-dir-symlink-int-1 \
	pkg-dir-file-moved pkg-dir-symlink-ext-0 pkg-dir-symlink-ext-1

include ../Test.mk

build-hook:
	ln -sfT test-dir pkg-dir-conffile-1/test-path
	ln -sfT test-dir pkg-dir-symlink-ext-1/test-path
	ln -sfT test-dir pkg-dir-symlink-int-1/test-path

clean-hook:
	$(RM) pkg-dir-conffile-1/test-path
	$(RM) pkg-dir-symlink-ext-1/test-path
	$(RM) pkg-dir-symlink-int-1/test-path

test-case:
	# Cannot switch a directory containing conffiles
	$(DPKG_INSTALL) pkg-dir-conffile-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! $(DPKG_INSTALL) pkg-dir-conffile-1.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_PURGE) pkg-dir-conffile

	# Cannot switch a directory containing files owned by another package
	$(DPKG_INSTALL) pkg-dir-file-other.deb
	$(DPKG_INSTALL) pkg-dir-symlink-int-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! test -h '$(DPKG_INSTDIR)/test-path'
	! $(DPKG_INSTALL) pkg-dir-symlink-int-1.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_PURGE) pkg-dir-symlink-int
	$(DPKG_PURGE) pkg-dir-file-other

	# Cannot switch a directory containing local files
	$(DPKG_INSTALL) pkg-dir-symlink-int-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! test -h '$(DPKG_INSTDIR)/test-path'
	touch '$(DPKG_INSTDIR)/test-path/local file'
	! $(DPKG_INSTALL) pkg-dir-symlink-int-1.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! test -h '$(DPKG_INSTDIR)/test-path'
	rm '$(DPKG_INSTDIR)/test-path/local file'
	$(DPKG_PURGE) pkg-dir-symlink-int

	#
	# Otherwise allow the switch
	#

	# Switch a directory to a symlink, contents moved within the package,
	# with intermediate full installs.
	$(DPKG_INSTALL) pkg-dir-symlink-int-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	! test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_INSTALL) pkg-dir-symlink-int-1.deb
	test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_PURGE) pkg-dir-symlink-int
	! test -e '$(DPKG_INSTDIR)/test-path'

	# Switch a directory to a symlink, contents moved within the package,
	# with full install, upgrade with unpack and configure.
	$(DPKG_INSTALL) pkg-dir-symlink-int-0.deb
	! test -h '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_UNPACK) pkg-dir-symlink-int-1.deb
	! test -h '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_CONFIGURE) pkg-dir-symlink-int
	test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_PURGE) pkg-dir-symlink-int
	! test -e '$(DPKG_INSTDIR)/test-path'

	# Switch a directory to a symlink, contents moved within the package,
	# with two unpacks and configure.
	$(DPKG_UNPACK) pkg-dir-symlink-int-0.deb
	! test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_UNPACK) pkg-dir-symlink-int-1.deb
	! test -h '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_CONFIGURE) pkg-dir-symlink-int
	test -h '$(DPKG_INSTDIR)/test-path'
	$(DPKG_PURGE) pkg-dir-symlink-int
	! test -e '$(DPKG_INSTDIR)/test-path'

	# Switch a directory to a symlink, contents moved to another package,
	# with intermediate full installs.
	$(DPKG_INSTALL) pkg-dir-symlink-ext-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_INSTALL) pkg-dir-file-moved.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_INSTALL) pkg-dir-symlink-ext-1.deb
	test -h '$(DPKG_INSTDIR)/test-path'
	test "`readlink -f '$(DPKG_INSTDIR)/test-path'`" = "/test-dir"
	! test -h '$(DPKG_INSTDIR)/test-dir'
	test -d '$(DPKG_INSTDIR)/test-dir'
	test -f '$(DPKG_INSTDIR)/test-dir/file-normal'
	$(DPKG_PURGE) pkg-dir-symlink-ext
	$(DPKG_PURGE) pkg-dir-file-moved

	# Switch a directory to a symlink, contents moved to another package,
	# with full installs, upgrade with unpack and configure.
	$(DPKG_INSTALL) pkg-dir-symlink-ext-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_INSTALL) pkg-dir-file-moved.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_UNPACK) pkg-dir-symlink-ext-1.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_CONFIGURE) -a
	test -h '$(DPKG_INSTDIR)/test-path'
	test "`readlink -f '$(DPKG_INSTDIR)/test-path'`" = "/test-dir"
	! test -h '$(DPKG_INSTDIR)/test-dir'
	test -d '$(DPKG_INSTDIR)/test-dir'
	test -f '$(DPKG_INSTDIR)/test-dir/file-normal'
	$(DPKG_PURGE) pkg-dir-symlink-ext
	$(DPKG_PURGE) pkg-dir-file-moved

	# Switch a directory to a symlink, contents moved to another package,
	# with three unpacks and configure.
	$(DPKG_UNPACK) pkg-dir-symlink-ext-0.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	$(DPKG_UNPACK) pkg-dir-file-moved.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_UNPACK) pkg-dir-symlink-ext-1.deb
	test -d '$(DPKG_INSTDIR)/test-path'
	test -d '$(DPKG_INSTDIR)/test-dir'
	$(DPKG_CONFIGURE) -a
	test -h '$(DPKG_INSTDIR)/test-path'
	test "`readlink -f '$(DPKG_INSTDIR)/test-path'`" = "/test-dir"
	! test -h '$(DPKG_INSTDIR)/test-dir'
	test -d '$(DPKG_INSTDIR)/test-dir'
	test -f '$(DPKG_INSTDIR)/test-dir/file-normal'
	$(DPKG_PURGE) pkg-dir-symlink-ext
	$(DPKG_PURGE) pkg-dir-file-moved

test-clean:
	$(DPKG_PURGE) pkg-dir-conffile
	$(DPKG_PURGE) pkg-dir-symlink-int
	$(DPKG_PURGE) pkg-dir-symlink-ext
	$(DPKG_PURGE) pkg-dir-file-other
	$(DPKG_PURGE) pkg-dir-file-moved
