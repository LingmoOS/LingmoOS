################################################################################
#
# libsigsegv
#
################################################################################

LIBSIGSEGV_VERSION = 2.14
LIBSIGSEGV_SITE = $(LINGMO_GNU_MIRROR)/libsigsegv
LIBSIGSEGV_INSTALL_STAGING = YES
LIBSIGSEGV_CONF_ENV = sv_cv_fault_posix=yes
LIBSIGSEGV_LICENSE = GPL-2.0+
LIBSIGSEGV_LICENSE_FILES = COPYING
# We're patching configure.ac
LIBSIGSEGV_AUTORECONF = YES

$(eval $(autotools-package))
