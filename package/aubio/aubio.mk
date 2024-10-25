################################################################################
#
# aubio
#
################################################################################

AUBIO_VERSION = 0.4.9
AUBIO_SITE = https://aubio.org/pub
AUBIO_SOURCE = aubio-$(AUBIO_VERSION).tar.bz2
AUBIO_LICENSE = GPL-3.0+
AUBIO_LICENSE_FILES = COPYING
AUBIO_CPE_ID_VENDOR = aubio
AUBIO_INSTALL_STAGING = YES

AUBIO_DEPENDENCIES = host-pkgconf
AUBIO_CONF_OPTS = \
	--disable-docs \
	--disable-atlas

# The waf script bundled in aubio 0.4.9 is too old for python3.11
# Similar issue with Jack:
# https://github.com/jackaudio/jack2/issues/898
AUBIO_NEEDS_EXTERNAL_WAF = YES

# Add --notests for each build step to avoid running unit tests on the
# build machine.
AUBIO_WAF_OPTS = --notests

ifeq ($(LINGMO_PACKAGE_LIBSNDFILE),y)
AUBIO_DEPENDENCIES += libsndfile
AUBIO_CONF_OPTS += --enable-sndfile
else
AUBIO_CONF_OPTS += --disable-sndfile
endif

# Could not compile aubio in double precision mode with libsamplerate
ifeq ($(LINGMO_PACKAGE_LIBSAMPLERATE):$(LINGMO_PACKAGE_FFTW_DOUBLE),y:)
AUBIO_DEPENDENCIES += libsamplerate
AUBIO_CONF_OPTS += --enable-samplerate
else
AUBIO_CONF_OPTS += --disable-samplerate
endif

ifeq ($(LINGMO_PACKAGE_JACK2),y)
AUBIO_DEPENDENCIES += jack2
AUBIO_CONF_OPTS += --enable-jack
else
AUBIO_CONF_OPTS += --disable-jack
endif

# fftw3 require double otherwise it will look for fftw3f
ifeq ($(LINGMO_PACKAGE_FFTW_DOUBLE),y)
AUBIO_CONF_OPTS += --enable-fftw3 --enable-double
AUBIO_DEPENDENCIES += fftw-double
else ifeq ($(LINGMO_PACKAGE_FFTW_SINGLE),y)
AUBIO_CONF_OPTS += --enable-fftw3f --disable-double
AUBIO_DEPENDENCIES += fftw-single
else
AUBIO_CONF_OPTS += --disable-fftw3
endif

ifeq ($(LINGMO_PACKAGE_FFMPEG_AVRESAMPLE),y)
AUBIO_DEPENDENCIES += ffmpeg
AUBIO_CONF_OPTS += --enable-avcodec
else
AUBIO_CONF_OPTS += --disable-avcodec
endif

$(eval $(waf-package))
