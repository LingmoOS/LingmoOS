PKG := pkg-hardlink

TESTS_DEB := $(PKG)

include ../Test.mk

$(PKG)/test-conffile-link0:
	echo "test hard linked conffile" >$@
	link $@ $(PKG)/test-conffile-link1

$(PKG)/test-file-link0:
	echo "test hard linked file" >$@
	link $@ $(PKG)/test-file-link1

$(PKG)/test-fifo-link0:
	mkfifo $@
	link $@ $(PKG)/test-fifo-link1

$(PKG)/test-null-link0:
	$(BEROOT) mknod $@ c 1 3
	$(BEROOT) link $@ $(PKG)/test-null-link1

$(PKG)/test-ram0-link0:
	$(BEROOT) mknod $@ b 1 0
	$(BEROOT) link $@ $(PKG)/test-ram0-link1

TEST_LINKS := \
	$(PKG)/test-conffile-link0 \
	$(PKG)/test-file-link0 \
	$(PKG)/test-fifo-link0 \
	# EOL

ifdef DPKG_AS_ROOT
TEST_LINKS += \
	$(PKG)/test-null-link0 \
	$(PKG)/test-ram0-link0 \
	# EOL
endif

build-hook: $(TEST_LINKS)

clean-hook:
	rm -f $(PKG)/test-conffile-link0 $(PKG)/test-conffile-link1
	rm -f $(PKG)/test-file-link0 $(PKG)/test-file-link1
	rm -f $(PKG)/test-fifo-link0 $(PKG)/test-fifo-link1
	rm -f $(PKG)/test-null-link0 $(PKG)/test-null-link1
	rm -f $(PKG)/test-ram0-link0 $(PKG)/test-ram0-link1

test-case:
	# test unpacking hardlinks
	$(DPKG_INSTALL) pkg-hardlink.deb
	$(call pkg_is_installed,pkg-hardlink)

test-clean:
	$(DPKG_PURGE) pkg-hardlink

