include ../Test.mk

test-case:
	# test building package with bogus control files
	! $(DPKG_BUILD_DEB) pkg-bogus-colon
	! $(DPKG_BUILD_DEB) pkg-bogus-blank
	! $(DPKG_BUILD_DEB) pkg-bogus-fieldname-empty
	! $(DPKG_BUILD_DEB) pkg-bogus-fieldname-partial
	! $(DPKG_BUILD_DEB) pkg-bogus-format-string
	
	# build bogus packages
	$(DPKG_BUILD_DEB) --no-check pkg-bogus-colon
	$(DPKG_BUILD_DEB) --no-check pkg-bogus-blank
	$(DPKG_BUILD_DEB) --no-check pkg-bogus-fieldname-empty
	$(DPKG_BUILD_DEB) --no-check pkg-bogus-fieldname-partial
	$(DPKG_BUILD_DEB) --no-check pkg-bogus-format-string
	
	# test installing packages with bogus control files
	! $(DPKG_INSTALL) pkg-bogus-colon.deb
	$(call pkg_is_not_installed,pkg-bogus-colon)
	! $(DPKG_INSTALL) pkg-bogus-blank.deb
	$(call pkg_is_not_installed,pkg-bogus-blank)
	! $(DPKG_INSTALL) pkg-bogus-fieldname-empty.deb
	$(call pkg_is_not_installed,pkg-bogus-fieldname-empty)
	! $(DPKG_INSTALL) pkg-bogus-fieldname-partial.deb
	$(call pkg_is_not_installed,pkg-bogus-fieldname-partial)
	! $(DPKG_INSTALL) pkg-bogus-format-string.deb
	$(call pkg_is_not_installed,pkg-bogus-format-string)

test-clean:
	rm -f pkg-bogus-colon.deb
	rm -f pkg-bogus-blank.deb
