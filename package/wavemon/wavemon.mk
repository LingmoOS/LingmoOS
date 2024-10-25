################################################################################
#
# wavemon
#
################################################################################

WAVEMON_VERSION = 0.9.5
WAVEMON_SITE = $(call github,uoaerg,wavemon,v$(WAVEMON_VERSION))
WAVEMON_LICENSE = GPL-3.0+
WAVEMON_LICENSE_FILES = LICENSE
WAVEMON_DEPENDENCIES = host-pkgconf libnl ncurses

# Handwritten Makefile.in, automake isn't used
WAVEMON_MAKE_OPTS = CC="$(TARGET_CC)"

ifeq ($(LINGMO_PACKAGE_LIBCAP),y)
WAVEMON_CONF_OPTS += --with-libcap
WAVEMON_DEPENDENCIES += libcap
else
WAVEMON_CONF_OPTS += --without-libcap
endif

$(eval $(autotools-package))
