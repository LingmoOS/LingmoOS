################################################################################
#
# rdesktop
#
################################################################################

RDESKTOP_VERSION = 1.9.0
RDESKTOP_SITE = \
	https://github.com/rdesktop/rdesktop/releases/download/v$(RDESKTOP_VERSION)
RDESKTOP_DEPENDENCIES = \
	host-pkgconf \
	gnutls \
	libtasn1 \
	nettle \
	xlib_libX11 \
	xlib_libXcursor \
	xlib_libXt \
	$(if $(LINGMO_PACKAGE_ALSA_LIB_PCM),alsa-lib) \
	$(if $(LINGMO_PACKAGE_LIBAO),libao) \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv) \
	$(if $(LINGMO_PACKAGE_LIBSAMPLERATE),libsamplerate) \
	$(if $(LINGMO_PACKAGE_PULSEAUDIO),pulseaudio) \
	$(if $(LINGMO_PACKAGE_XLIB_LIBXRANDR),xlib_libXrandr)
RDESKTOP_CONF_OPTS = --disable-credssp
RDESKTOP_LICENSE = GPL-3.0+
RDESKTOP_LICENSE_FILES = COPYING
RDESKTOP_CPE_ID_VENDOR = rdesktop

ifeq ($(LINGMO_PACKAGE_PCSC_LITE),y)
RDESKTOP_DEPENDENCIES += pcsc-lite
else
RDESKTOP_CONF_OPTS += --disable-smartcard
endif

$(eval $(autotools-package))
