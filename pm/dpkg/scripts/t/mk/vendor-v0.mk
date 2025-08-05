dpkg_vendor_derives_from = $(dpkg_vendor_derives_from_v0)

include $(srcdir)/mk/vendor.mk

test:
	: # Test the dpkg_vendor_derives_from v0 macro.
	test "$(shell $(call dpkg_vendor_derives_from,debian))" = "yes"
