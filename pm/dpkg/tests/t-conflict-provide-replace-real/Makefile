TESTS_DEB := pkg-a pkg-b

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-a.deb
	# test if the replacing package takes over on install
	$(DPKG_INSTALL) pkg-b.deb

test-clean:
	$(DPKG_PURGE) pkg-b
	-$(DPKG_PURGE) pkg-a

