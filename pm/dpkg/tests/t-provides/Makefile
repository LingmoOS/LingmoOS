TESTS_DEB := \
	pkg-provides pkg-provides-10 pkg-provides-20 pkg-provides-10-20 \
	pkg-depends pkg-depends-eq pkg-depends-le pkg-depends-ge \
	pkg-predepends pkg-predepends-eq pkg-predepends-le pkg-predepends-ge \
	pkg-breaks pkg-breaks-eq pkg-breaks-le pkg-breaks-ge \
	pkg-conflicts pkg-conflicts-eq pkg-conflicts-le pkg-conflicts-ge \
	pkg-a-depends-ge-lt pkg-b-provides-10 pkg-c-provides-20

include ../Test.mk

test-bogus:
	# test building with bogus provides
	! $(DPKG_BUILD_DEB) pkg-provides-bogus

	# build bogus packages
	$(DPKG_BUILD_DEB) --no-check pkg-provides-bogus

	# test installing packages with bogus provides
	! $(DPKG_INSTALL) pkg-provides-bogus.deb
	$(call pkg_is_not_installed,pkg-provides-bogus)

define PURGE_DEPENDS
$(DPKG_PURGE) pkg-depends
$(DPKG_PURGE) pkg-depends-le
$(DPKG_PURGE) pkg-depends-eq
$(DPKG_PURGE) pkg-depends-ge
endef

test-depends:
	# The provided dependency is not present, fail
	! $(DPKG_INSTALL) pkg-depends.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-ge.deb

	# Install the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	$(DPKG_INSTALL) pkg-depends.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-ge.deb
	# Upgrade the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	! $(DPKG_PURGE) pkg-provides

	$(PURGE_DEPENDS)

	# Install the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb
	$(DPKG_INSTALL) pkg-depends.deb
	$(DPKG_INSTALL) pkg-depends-le.deb
	! $(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-ge.deb
	# Upgrade the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb

	$(PURGE_DEPENDS)

	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb
	$(DPKG_INSTALL) pkg-depends.deb
	$(DPKG_INSTALL) pkg-depends-eq.deb
	! $(DPKG_INSTALL) pkg-depends-le.deb
	$(DPKG_INSTALL) pkg-depends-ge.deb
	# Upgrade the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb

	$(PURGE_DEPENDS)

	# Install the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb
	$(DPKG_INSTALL) pkg-depends.deb
	$(DPKG_INSTALL) pkg-depends-eq.deb
	$(DPKG_INSTALL) pkg-depends-le.deb
	$(DPKG_INSTALL) pkg-depends-ge.deb
	# Upgrade the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb

	$(PURGE_DEPENDS)

	$(DPKG_PURGE) pkg-provides

	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-c-provides-20.deb
	! $(DPKG_INSTALL) pkg-a-depends-ge-lt.deb
	$(DPKG_PURGE) pkg-a-depends-ge-lt
	$(DPKG_PURGE) pkg-c-provides

	# Install the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-b-provides-10.deb
	$(DPKG_INSTALL) pkg-a-depends-ge-lt.deb
	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-c-provides-20.deb

	# Cannot purge the only package satisfying the dependency
	! $(DPKG_PURGE) pkg-b-provides
	# Can purge package not satisfying the dependency
	$(DPKG_PURGE) pkg-c-provides

	$(DPKG_PURGE) pkg-a-depends-ge-lt
	$(DPKG_PURGE) pkg-b-provides

	# The provided dependency is removed, pass
	$(DPKG_PURGE) pkg-provides

define PURGE_PREDEPENDS
$(DPKG_PURGE) pkg-predepends
$(DPKG_PURGE) pkg-predepends-le
$(DPKG_PURGE) pkg-predepends-eq
$(DPKG_PURGE) pkg-predepends-ge
endef

test-predepends:
	# The provided predependency is not present, fail
	! $(DPKG_UNPACK) pkg-predepends.deb
	! $(DPKG_UNPACK) pkg-predepends-eq.deb
	! $(DPKG_UNPACK) pkg-predepends-le.deb
	! $(DPKG_UNPACK) pkg-predepends-ge.deb

	# Install the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	$(DPKG_UNPACK) pkg-predepends.deb
	! $(DPKG_UNPACK) pkg-predepends-eq.deb
	! $(DPKG_UNPACK) pkg-predepends-le.deb
	! $(DPKG_UNPACK) pkg-predepends-ge.deb
	# Upgrade the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	! $(DPKG_PURGE) pkg-provides

	$(PURGE_PREDEPENDS)

	# Install the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb
	$(DPKG_UNPACK) pkg-predepends.deb
	$(DPKG_UNPACK) pkg-predepends-le.deb
	! $(DPKG_UNPACK) pkg-predepends-eq.deb
	! $(DPKG_UNPACK) pkg-predepends-ge.deb
	# Upgrade the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb

	$(PURGE_PREDEPENDS)

	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb
	$(DPKG_UNPACK) pkg-predepends.deb
	$(DPKG_UNPACK) pkg-predepends-eq.deb
	! $(DPKG_UNPACK) pkg-predepends-le.deb
	$(DPKG_UNPACK) pkg-predepends-ge.deb
	# Upgrade the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb

	$(PURGE_PREDEPENDS)

	# Install the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb
	$(DPKG_UNPACK) pkg-predepends.deb
	$(DPKG_UNPACK) pkg-predepends-eq.deb
	$(DPKG_UNPACK) pkg-predepends-le.deb
	$(DPKG_UNPACK) pkg-predepends-ge.deb
	# Upgrade the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb

	$(PURGE_PREDEPENDS)

	# The provided predependency is removed, pass
	$(DPKG_PURGE) pkg-provides

define PURGE_BREAKS
$(DPKG_PURGE) pkg-breaks
$(DPKG_PURGE) pkg-breaks-le
$(DPKG_PURGE) pkg-breaks-eq
$(DPKG_PURGE) pkg-breaks-ge
endef

test-breaks:
	# The provided dependency is not present, pass
	$(DPKG_INSTALL) pkg-breaks.deb
	$(DPKG_INSTALL) pkg-breaks-eq.deb
	$(DPKG_INSTALL) pkg-breaks-le.deb
	$(DPKG_INSTALL) pkg-breaks-ge.deb

	$(PURGE_BREAKS)

	# Install the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	! $(DPKG_INSTALL) pkg-breaks.deb
	$(DPKG_INSTALL) pkg-breaks-eq.deb
	$(DPKG_INSTALL) pkg-breaks-le.deb
	$(DPKG_INSTALL) pkg-breaks-ge.deb
	# Upgrade the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	$(DPKG_PURGE) pkg-provides

	$(PURGE_BREAKS)

	# Install the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb
	! $(DPKG_INSTALL) pkg-breaks.deb
	! $(DPKG_INSTALL) pkg-breaks-le.deb
	$(DPKG_INSTALL) pkg-breaks-eq.deb
	$(DPKG_INSTALL) pkg-breaks-ge.deb
	# Upgrade the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb

	$(PURGE_BREAKS)

	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb
	! $(DPKG_INSTALL) pkg-breaks.deb
	! $(DPKG_INSTALL) pkg-breaks-eq.deb
	$(DPKG_INSTALL) pkg-breaks-le.deb
	! $(DPKG_INSTALL) pkg-breaks-ge.deb
	# Upgrade the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb

	$(PURGE_BREAKS)

	# Install the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb
	! $(DPKG_INSTALL) pkg-breaks.deb
	! $(DPKG_INSTALL) pkg-breaks-eq.deb
	! $(DPKG_INSTALL) pkg-breaks-le.deb
	! $(DPKG_INSTALL) pkg-breaks-ge.deb
	# Upgrade the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb

	$(PURGE_BREAKS)

	# The provided dependency is removed, pass
	$(DPKG_PURGE) pkg-provides

define PURGE_CONFLICTS
$(DPKG_PURGE) pkg-conflicts
$(DPKG_PURGE) pkg-conflicts-le
$(DPKG_PURGE) pkg-conflicts-eq
$(DPKG_PURGE) pkg-conflicts-ge
endef

test-conflicts:
	# The provided predependency is not present, fail
	$(DPKG_UNPACK) pkg-conflicts.deb
	$(DPKG_UNPACK) pkg-conflicts-eq.deb
	$(DPKG_UNPACK) pkg-conflicts-le.deb
	$(DPKG_UNPACK) pkg-conflicts-ge.deb

	$(PURGE_CONFLICTS)

	# Install the providing package (versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	! $(DPKG_UNPACK) pkg-conflicts.deb
	$(DPKG_UNPACK) pkg-conflicts-eq.deb
	$(DPKG_UNPACK) pkg-conflicts-le.deb
	$(DPKG_UNPACK) pkg-conflicts-ge.deb
	# Upgrade the providing package (pkg-virtual versionless)
	$(DPKG_INSTALL) pkg-provides.deb
	$(DPKG_PURGE) pkg-provides

	$(PURGE_CONFLICTS)

	# Install the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb
	! $(DPKG_UNPACK) pkg-conflicts.deb
	! $(DPKG_UNPACK) pkg-conflicts-le.deb
	$(DPKG_UNPACK) pkg-conflicts-eq.deb
	$(DPKG_UNPACK) pkg-conflicts-ge.deb
	# Upgrade the providing package (pkg-virtual 10)
	$(DPKG_INSTALL) pkg-provides-10.deb

	$(PURGE_CONFLICTS)

	# Install the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb
	! $(DPKG_UNPACK) pkg-conflicts.deb
	! $(DPKG_UNPACK) pkg-conflicts-eq.deb
	$(DPKG_UNPACK) pkg-conflicts-le.deb
	! $(DPKG_UNPACK) pkg-conflicts-ge.deb
	# Upgrade the providing package (pkg-virtual 20)
	$(DPKG_INSTALL) pkg-provides-20.deb

	$(PURGE_CONFLICTS)

	# Install the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb
	! $(DPKG_UNPACK) pkg-conflicts.deb
	! $(DPKG_UNPACK) pkg-conflicts-eq.deb
	! $(DPKG_UNPACK) pkg-conflicts-le.deb
	! $(DPKG_UNPACK) pkg-conflicts-ge.deb
	# Upgrade the providing package (pkg-virtual 10 && 20)
	$(DPKG_INSTALL) pkg-provides-10-20.deb

	$(PURGE_CONFLICTS)

	# The provided predependency is removed, pass
	$(DPKG_PURGE) pkg-provides

test-case: test-bogus test-depends test-predepends test-breaks test-conflicts

test-clean:
	-$(DPKG_PURGE) pkg-depends
	-$(DPKG_PURGE) pkg-depends-eq
	-$(DPKG_PURGE) pkg-depends-ge
	-$(DPKG_PURGE) pkg-depends-le
	-$(DPKG_PURGE) pkg-predepends
	-$(DPKG_PURGE) pkg-predepends-eq
	-$(DPKG_PURGE) pkg-predepends-ge
	-$(DPKG_PURGE) pkg-predepends-le
	-$(DPKG_PURGE) pkg-breaks
	-$(DPKG_PURGE) pkg-breaks-eq
	-$(DPKG_PURGE) pkg-breaks-ge
	-$(DPKG_PURGE) pkg-breaks-le
	-$(DPKG_PURGE) pkg-conflicts
	-$(DPKG_PURGE) pkg-conflicts-eq
	-$(DPKG_PURGE) pkg-conflicts-ge
	-$(DPKG_PURGE) pkg-conflicts-le
	-$(DPKG_PURGE) pkg-provides
	-$(DPKG_PURGE) pkg-a-depends-ge-lt
	-$(DPKG_PURGE) pkg-b-provides
	-$(DPKG_PURGE) pkg-c-provides

	rm -f pkg-provides-bogus.deb
