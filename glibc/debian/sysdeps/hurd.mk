# This is for the GNU OS.  Commonly known as the Hurd.
libc = libc0.3

# Build and expect pt_chown on this platform
pt_chown = yes
# Expect pldd on this platform
pldd = no

# Linuxthreads Config (we claim "no threads" as nptl keys off this)
threads = no
# MIG generates a lot of warnings
extra_config_options = --disable-werror

ifndef HURD_SOURCE
  HURD_HEADERS := /usr/include
else
  HURD_HEADERS := $(HURD_SOURCE)/include
endif

# Minimum Kernel supported
with_headers = --with-headers=$(shell pwd)/debian/include

KERNEL_HEADER_DIR = $(stamp)mkincludedir
$(stamp)mkincludedir:
	rm -rf debian/include
	mkdir debian/include

	# System headers
	for path in hurd mach mach_debug device cthreads.h; do \
	    if [ -e "$(HURD_HEADERS)/$(DEB_HOST_MULTIARCH)/$$path" ]; then \
	        ln -s $(HURD_HEADERS)/$(DEB_HOST_MULTIARCH)/$$path debian/include/$$path ; \
	    elif [ -e "$(HURD_HEADERS)/$$path" ]; then \
	        ln -s $(HURD_HEADERS)/$$path debian/include/$$path ; \
	    fi ; \
	done

	# To make configure happy if libc0.3-dev is not installed.
	touch debian/include/assert.h

	touch $@

# Also to make configure happy.
export CPPFLAGS = -isystem $(shell pwd)/debian/include

# We prefer easy backtraces over small performance hit
extra_cflags = -fno-omit-frame-pointer

# Do not care about kernel versions for now.
define kernel_check
true
endef
