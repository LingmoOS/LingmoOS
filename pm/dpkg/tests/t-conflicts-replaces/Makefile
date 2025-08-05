TESTS_DEB := pkg-a pkg-b

include ../Test.mk

test-case:
	# test if the conflicting package replaces on install
	$(DPKG_INSTALL) pkg-a.deb
	$(DPKG_INSTALL) pkg-b.deb
	$(call pkg_is_installed,pkg-b)
	$(call pkg_is_not_installed,pkg-a)
	-$(DPKG_PURGE) pkg-a pkg-b

	# test if the conflicting package replaces on install
	$(DPKG_INSTALL) pkg-a.deb pkg-b.deb
	$(call pkg_is_installed,pkg-b)
	$(call pkg_is_not_installed,pkg-a)
	-$(DPKG_PURGE) pkg-a pkg-b

	# test if the conflicting package fails on install
	$(DPKG_INSTALL) pkg-b.deb
	! $(DPKG_INSTALL) pkg-a.deb
	$(call pkg_is_not_installed,pkg-a)
	$(call pkg_is_installed,pkg-b)
	-$(DPKG_PURGE) pkg-a pkg-b

	# test if the conflicting package fails on install
	! $(DPKG_INSTALL) pkg-b.deb pkg-a.deb
	$(call pkg_is_not_installed,pkg-a)
	-$(DPKG_PURGE) pkg-a pkg-b

test-clean:
	-$(DPKG_PURGE) pkg-a pkg-b

