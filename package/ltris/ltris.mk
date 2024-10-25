################################################################################
#
# ltris
#
################################################################################

LTRIS_SITE = http://downloads.sourceforge.net/lgames/ltris
LTRIS_VERSION = 1.2.7
LTRIS_LICENSE = GPL-2.0+
LTRIS_LICENSE_FILES = COPYING

LTRIS_DEPENDENCIES = sdl $(TARGET_NLS_DEPENDENCIES)
LTRIS_LIBS = $(TARGET_NLS_LIBS)

LTRIS_CONF_ENV = \
	SDL_CONFIG="$(STAGING_DIR)/usr/bin/sdl-config" \
	LIBS="$(LTRIS_LIBS)"

ifeq ($(LINGMO_PACKAGE_LTRIS_AUDIO),y)
LTRIS_DEPENDENCIES += sdl_mixer host-pkgconf
# configure script does NOT use pkg-config to figure out how to link
# with sdl_mixer, breaking static linking as sdl_mixer can use libmad
LTRIS_LIBS += `$(PKG_CONFIG_HOST_BINARY) --libs SDL_mixer`
endif

$(eval $(autotools-package))
