TESTS_DEB := pkg-a pkg-b

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-a.deb
	# The user was not explicit in the request, fail
	! $(DPKG_INSTALL) pkg-b.deb
	# Even if requesting auto-deconfigure the request is not explicit
	# enough, fail
	! $(DPKG_INSTALL) -B pkg-b.deb
	# Set up the desired transaction
	echo pkg-a deinstall | $(BEROOT) $(DPKG) --set-selections
	$(DPKG_INSTALL) pkg-b.deb

test-clean:
	$(DPKG_PURGE) pkg-b
	-$(DPKG_PURGE) pkg-a
