TESTS_DEB := pkg-conff-prompt-0 pkg-conff-prompt-1

include ../Test.mk

build-hook:
	touch "pkg-conff-prompt-0/test-conffile with spaces"
	touch "pkg-conff-prompt-1/test-conffile with spaces"

clean-hook:
	$(RM) "pkg-conff-prompt-0/test-conffile with spaces"
	$(RM) "pkg-conff-prompt-1/test-conffile with spaces"

test-case:
	$(DPKG_INSTALL) pkg-conff-prompt-0.deb
	# test if package prompts on install
	$(DPKG_INSTALL) pkg-conff-prompt-1.deb

test-clean:
	$(DPKG_PURGE) pkg-conff-prompt

