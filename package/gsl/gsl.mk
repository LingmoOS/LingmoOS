################################################################################
#
# gsl
#
################################################################################

GSL_VERSION = 2.7.1
GSL_SITE = $(LINGMO_GNU_MIRROR)/gsl
GSL_INSTALL_STAGING = YES
GSL_LICENSE = GPL-3.0
GSL_LICENSE_FILES = COPYING
GSL_CONFIG_SCRIPTS = gsl-config
# We're patching configure.ac
GSL_AUTORECONF = YES

# uClibc pretends to have fenv support as it installs <fenv.h>, but in
# practice, it only implements it for i386. Problem reported upstream
# at: http://lists.busybox.net/pipermail/uclibc/2012-October/047067.html.
# So we tell gsl that fenv related functions are not available in this
# case.
ifeq ($(LINGMO_TOOLCHAIN_USES_UCLIBC),y)
ifneq ($(LINGMO_i386),y)
GSL_CONF_ENV = \
	ac_cv_have_decl_feenableexcept=no \
	ac_cv_have_decl_fesettrapenable=no
endif
endif

$(eval $(autotools-package))
