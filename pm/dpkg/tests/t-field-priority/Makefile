TESTS_DEB := pkg-priority

include ../Test.mk

test-case:
	# test for unknown priority values
	$(DPKG_INSTALL) pkg-priority.deb
	$(call pkg_is_installed,pkg-priority)
	$(call pkg_field_is,pkg-priority,Priority,value not known)

test-clean:
	$(DPKG_PURGE) pkg-priority
