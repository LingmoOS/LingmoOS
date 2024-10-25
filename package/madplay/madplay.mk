################################################################################
#
# madplay
#
################################################################################

MADPLAY_VERSION = 0.15.2b
MADPLAY_SITE = http://downloads.sourceforge.net/project/mad/madplay/$(MADPLAY_VERSION)
MADPLAY_LICENSE = GPL-2.0+
MADPLAY_LICENSE_FILES = COPYING COPYRIGHT
MADPLAY_DEPENDENCIES = host-pkgconf libmad libid3tag $(TARGET_NLS_DEPENDENCIES)

# Force autoreconf to be able to use a more recent libtool script, that
# is able to properly behave in the face of a missing C++ compiler.
# Also 0003-configure-ac-use-pkg-config-to-find-id3tag.patch
MADPLAY_AUTORECONF = YES

# madplay uses libmad which has some assembly function that is not present in
# Thumb mode:
# Error: selected processor does not support `smull r8,r9,r2,r4' in Thumb mode
# so, we desactivate Thumb mode
ifeq ($(LINGMO_ARM_INSTRUCTIONS_THUMB),y)
MADPLAY_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -marm"
endif

# Check if ALSA is built, then we should configure after alsa-lib so
# ./configure can find alsa-lib.
ifeq ($(LINGMO_PACKAGE_MADPLAY_ALSA),y)
MADPLAY_CONF_OPTS += --with-alsa
MADPLAY_DEPENDENCIES += alsa-lib
MADPLAY_CONF_ENV += LIBS="`$(PKG_CONFIG_HOST_BINARY) --libs alsa`"
endif

$(eval $(autotools-package))
