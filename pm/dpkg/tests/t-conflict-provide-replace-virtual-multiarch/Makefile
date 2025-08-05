TESTS_DEB := pkg-virt-i386 pkg-virt-amd64 pkg-real-i386 pkg-real-amd64

include ../Test.mk

test-case:
	$(DPKG) --add-architecture amd64
	$(DPKG) --add-architecture i386
	$(DPKG_INSTALL) pkg-virt-i386.deb
	# Configuring pkg-real:amd64 should not be allowed
	! $(DPKG_INSTALL) pkg-real-amd64.deb
	$(DPKG_INSTALL) pkg-real-i386.deb
	$(DPKG_INSTALL) pkg-virt-amd64.deb
	$(DPKG_INSTALL) pkg-real-i386.deb
	# Purging should not be allowed
	! $(DPKG_PURGE) pkg-virt:i386

test-clean:
	$(DPKG_PURGE) pkg-real:i386
	$(DPKG_PURGE) pkg-real:amd64
	$(DPKG_PURGE) pkg-virt:i386
	$(DPKG_PURGE) pkg-virt:amd64
	$(DPKG) --remove-architecture amd64
	$(DPKG) --remove-architecture i386
