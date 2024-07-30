# Since version 2.28, glibc requires GNU Make >= 4.0
# https://www.sourceware.org/ml/libc-alpha/2018-08/msg00003.html
#
# Set this to either 4.0 or higher, depending on the highest minimum
# version required by any of the packages bundled in Buildroot. If a
# package is bumped or a new one added, and it requires a higher
# version, our package infra will catch it and whine.
#
LINGMO_MAKE_VERSION_MIN = 4.0

LINGMO_MAKE ?= $(call suitable-host-package,make,\
	$(LINGMO_MAKE_VERSION_MIN) $(MAKE))

ifeq ($(LINGMO_MAKE),)
LINGMO_MAKE = $(HOST_DIR)/bin/host-make -j$(PARALLEL_JOBS)
LINGMO_MAKE1 = $(HOST_DIR)/bin/host-make -j1
LINGMO_MAKE_HOST_DEPENDENCY = host-make
else
LINGMO_MAKE = $(MAKE)
LINGMO_MAKE1 = $(MAKE1)
endif
