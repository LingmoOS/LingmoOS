PKG := pkg-device

TESTS_DEB := $(PKG)

include ../Test.mk

$(PKG)/test-null:
	$(BEROOT) mknod $@ c 1 3

$(PKG)/test-ram0:
	$(BEROOT) mknod $@ b 1 0

build-hook: $(PKG)/test-null $(PKG)/test-ram0

clean-hook:
	rm -f $(PKG)/test-null
	rm -f $(PKG)/test-ram0

test-case:
	# test unpacking device files
	$(DPKG_INSTALL) pkg-device.deb
	$(call pkg_is_installed,pkg-device)

test-clean:
	$(DPKG_PURGE) pkg-device

