PKG := pkg-hardlink

TESTS_DEB := $(PKG)

include ../Test.mk

$(PKG)/test-hardlink:
	link $(PKG)/test-file $@

build-hook: $(PKG)/test-hardlink

clean-hook:
	rm -f $(PKG)/test-hardlink

test-case:
	$(DPKG_DIVERT_ADD) /test-file
	$(DPKG_DIVERT_ADD) /test-hardlink
	# test if unpack can divert a hardlink
	$(DPKG_INSTALL) pkg-hardlink.deb
	$(call pkg_is_installed,pkg-hardlink)
	test -f '$(DPKG_INSTDIR)/test-file.distrib'
	test -f '$(DPKG_INSTDIR)/test-hardlink.distrib'
	cmp '$(DPKG_INSTDIR)/test-file.distrib' \
	    '$(DPKG_INSTDIR)/test-hardlink.distrib'
	test "`stat -c '%i' '$(DPKG_INSTDIR)/test-file.distrib'`" = \
	     "`stat -c '%i' '$(DPKG_INSTDIR)/test-hardlink.distrib'`"

test-clean:
	$(DPKG_PURGE) pkg-hardlink
	$(DPKG_DIVERT_DEL) /test-file
	$(DPKG_DIVERT_DEL) /test-hardlink
