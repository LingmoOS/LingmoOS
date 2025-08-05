TESTS_DEB := pkg-interest pkg-trigger-conffile pkg-trigger-cmd

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-interest.deb

	# Test that the trigger gets activated on conffile installation
	$(DPKG_UNPACK) pkg-trigger-conffile.deb
	test -e "$(DPKG_INSTDIR)/test-triggered-cmd" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/test-triggered-cmd"
	$(DPKG_CONFIGURE) pkg-trigger-conffile
	test -e "$(DPKG_INSTDIR)/test-triggered-file" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/test-triggered-file"
	$(DPKG_PURGE) pkg-trigger-conffile

	# Test that the trigger gets activated on explicit file request
	$(DPKG_INSTALL) pkg-trigger-cmd.deb
	test -e "$(DPKG_INSTDIR)/test-triggered-cmd" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/test-triggered-cmd"
	$(DPKG_PURGE) pkg-trigger-cmd

	# Cleanup.
	$(DPKG_PURGE) pkg-interest

test-clean:
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/test-triggered-cmd" "$(DPKG_INSTDIR)/test-triggered-file"
	-$(DPKG_PURGE) pkg-interest
	-$(DPKG_PURGE) pkg-trigger-conffile pkg-trigger-cmd
