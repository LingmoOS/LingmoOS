TESTS_DEB := pkg-verify

include ../Test.mk

test-case:
	$(DPKG_INSTALL) pkg-verify.deb
	$(call pkg_is_installed,pkg-verify)
	$(call stdout_is,$(DPKG_VERIFY) pkg-verify,)
	$(BEROOT) sh -c "echo 'MODIFIED' >>'$(DPKG_INSTDIR)/test-dir/test-data'"
	$(call stdout_has,$(DPKG_VERIFY) pkg-verify,^\?\?5\?\?\?\?\?\?   /test-dir/test-data$)

test-clean:
	$(DPKG_PURGE) pkg-verify
