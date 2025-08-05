include $(srcdir)/mk/vendor.mk

dpkg_vendor_derives_from = $(dpkg_vendor_derives_from_v1)

test:
	: # Test the dpkg_vendor_derives_from v1 macro.
	test "$(call dpkg_vendor_derives_from,debian)" = "yes"
