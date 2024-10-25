# Since version 2.29, glibc requires python 3.4 or later to build the GNU C Library.
# https://www.sourceware.org/ml/libc-alpha/2019-01/msg00723.html

LINGMO_PYTHON3_VERSION_MIN = 3.4

ifeq (,$(call suitable-host-package,python3,$(LINGMO_PYTHON3_VERSION_MIN) python3 python))
LINGMO_PYTHON3_HOST_DEPENDENCY = host-python3
endif
