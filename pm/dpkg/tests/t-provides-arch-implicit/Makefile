TESTS_DEB := pkg-a pkg-b pkg-dep

include ../Test.mk

test-case:
	$(DPKG) --add-architecture i386
	$(DPKG) --add-architecture amd64
	$(DPKG_INSTALL) pkg-a.deb
	$(DPKG_INSTALL) pkg-b.deb
	$(DPKG_INSTALL) pkg-dep.deb
	! $(DPKG_PURGE) pkg-b
	$(DPKG_PURGE) pkg-a
	$(DPKG_PURGE) pkg-dep
	$(DPKG_PURGE) pkg-b

	$(DPKG_INSTALL) pkg-a.deb
	! $(DPKG_INSTALL) pkg-dep.deb
	$(DPKG_PURGE) pkg-a

test-clean:
	$(DPKG_PURGE) pkg-dep
	$(DPKG_PURGE) pkg-b pkg-b
	$(DPKG) --remove-architecture i386
	$(DPKG) --remove-architecture amd64
