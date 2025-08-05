TESTS_DEB := pkg-conff-original-0 pkg-conff-original-1 pkg-conff-takeover

include ../Test.mk

define VERIFY
$(call pkg_is_installed,pkg-conff-original)
$(call pkg_is_installed,pkg-conff-takeover)
test "`$(DPKG_QUERY) -S /test-conffile`" = "pkg-conff-takeover: /test-conffile"
cmp ref-conffile "$(DPKG_INSTDIR)/test-conffile"
endef

test-case:
	# Install packages
	$(DPKG_INSTALL) pkg-conff-original-1.deb
	$(DPKG_INSTALL) pkg-conff-takeover.deb
	# Downgrade to test versioned Replaces
	$(DPKG_INSTALL) pkg-conff-original-0.deb
	$(VERIFY)
	$(DPKG_PURGE) pkg-conff-original pkg-conff-takeover

