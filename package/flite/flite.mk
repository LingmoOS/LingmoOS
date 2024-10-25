################################################################################
#
# flite
#
################################################################################

FLITE_VERSION = 2.2
FLITE_SITE = $(call github,festvox,flite,v$(FLITE_VERSION))
FLITE_LICENSE = BSD-4-Clause
FLITE_LICENSE_FILES = COPYING
FLITE_CPE_ID_VENDOR = cmu

FLITE_INSTALL_STAGING = YES
# Patching configure.in
FLITE_AUTORECONF = YES
FLITE_DEPENDENCIES = host-pkgconf

# Sadly, Flite does not support parallel build, especially when building its
# shared libraries.
FLITE_MAKE = $(MAKE1)

ifeq ($(LINGMO_PACKAGE_ALSA_LIB),y)
FLITE_DEPENDENCIES += alsa-lib
FLITE_CONF_OPTS += --with-audio=alsa
else
FLITE_CONF_OPTS += --with-audio=oss
endif

$(eval $(autotools-package))
