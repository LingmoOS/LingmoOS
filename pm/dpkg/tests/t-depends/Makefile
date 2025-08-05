TESTS_DEB := \
	pkg-real pkg-depends \
	pkg-version-1 pkg-version-2 pkg-version-3 pkg-depends-eq \
	pkg-depends-lt pkg-depends-le pkg-depends-ge pkg-depends-gt

include ../Test.mk

test-depends:
	# The dependency is not present, fail
	! $(DPKG_INSTALL) pkg-depends.deb
	# Install the real package
	$(DPKG_INSTALL) pkg-real.deb
	# The dependency is present, pass
	$(DPKG_INSTALL) pkg-depends.deb
	# The dependency is removed, fail
	! $(DPKG_PURGE) pkg-real
	# The depending package is removed, pass
	$(DPKG_PURGE) pkg-depends
	# The dependency is removed, pass
	$(DPKG_PURGE) pkg-real
	# Now try with --force-depends
	$(DPKG_INSTALL) --force-depends pkg-depends.deb
	$(DPKG_PURGE) pkg-depends

define PURGE_DEPENDS
$(DPKG_PURGE) pkg-depends-lt
$(DPKG_PURGE) pkg-depends-le
$(DPKG_PURGE) pkg-depends-eq
$(DPKG_PURGE) pkg-depends-ge
$(DPKG_PURGE) pkg-depends-gt
endef

test-depends-versioned:
	$(PURGE_DEPENDS)

	# The dependency is not present, fail
	! $(DPKG_INSTALL) pkg-depends-lt.deb
	! $(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-ge.deb
	! $(DPKG_INSTALL) pkg-depends-gt.deb

	# Install the package v1
	$(DPKG_INSTALL) pkg-version-1.deb
	$(DPKG_INSTALL) pkg-depends-lt.deb
	$(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-ge.deb
	! $(DPKG_INSTALL) pkg-depends-gt.deb

	$(PURGE_DEPENDS)

	# Install the package v2
	$(DPKG_INSTALL) pkg-version-2.deb
	! $(DPKG_INSTALL) pkg-depends-lt.deb
	$(DPKG_INSTALL) pkg-depends-le.deb
	$(DPKG_INSTALL) pkg-depends-eq.deb
	$(DPKG_INSTALL) pkg-depends-ge.deb
	! $(DPKG_INSTALL) pkg-depends-gt.deb

	$(PURGE_DEPENDS)

	# Install the package v3
	$(DPKG_INSTALL) pkg-version-3.deb
	! $(DPKG_INSTALL) pkg-depends-lt.deb
	! $(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	$(DPKG_INSTALL) pkg-depends-ge.deb
	$(DPKG_INSTALL) pkg-depends-gt.deb

	$(PURGE_DEPENDS)

test-case: test-depends test-depends-versioned

test-clean:
	-$(DPKG_PURGE) pkg-depends
	-$(DPKG_PURGE) pkg-real
	-$(DPKG_PURGE) pkg-depends-lt
	-$(DPKG_PURGE) pkg-depends-le
	-$(DPKG_PURGE) pkg-depends-eq
	-$(DPKG_PURGE) pkg-depends-ge
	-$(DPKG_PURGE) pkg-depends-gt
	-$(DPKG_PURGE) pkg-version
