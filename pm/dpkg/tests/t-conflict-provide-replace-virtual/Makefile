TESTS_DEB := pkg-a pkg-b

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-a.deb
	# test if the conflicting package fails on install
	! $(DPKG_INSTALL) pkg-b.deb

test-clean:
	-$(DPKG_PURGE) pkg-b
	$(DPKG_PURGE) pkg-a

