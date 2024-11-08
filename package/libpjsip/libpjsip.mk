################################################################################
#
# libpjsip
#
################################################################################

LIBPJSIP_VERSION = 2.14
LIBPJSIP_SOURCE = pjproject-$(LIBPJSIP_VERSION).tar.gz
LIBPJSIP_SITE = $(call github,pjsip,pjproject,$(LIBPJSIP_VERSION))

LIBPJSIP_LICENSE = GPL-2.0+
LIBPJSIP_LICENSE_FILES = COPYING
LIBPJSIP_CPE_ID_VENDOR = teluu
LIBPJSIP_CPE_ID_PRODUCT = pjsip
LIBPJSIP_INSTALL_STAGING = YES
LIBPJSIP_MAKE = $(MAKE1)

LIBPJSIP_CFLAGS = $(TARGET_CFLAGS) -DPJ_HAS_IPV6=1

# relocation truncated to fit: R_68K_GOT16O
ifeq ($(LINGMO_m68k_cf),y)
LIBPJSIP_CFLAGS += -mxgot
endif

LIBPJSIP_CONF_ENV = \
	LD="$(TARGET_CC)" \
	CFLAGS="$(LIBPJSIP_CFLAGS)"

LIBPJSIP_CONF_OPTS = \
	--disable-resample \
	--disable-g7221-codec \
	--disable-ilbc-codec \
	--disable-libwebrtc \
	--disable-ext-sound \
	--disable-g711-codec \
	--disable-l16-codec \
	--disable-g722-codec \
	--disable-ipp \
	--disable-silk

# Note: aconfigure.ac is broken: --enable-epoll or --disable-epoll will
# both enable it. But that's OK, epoll is better than the alternative,
# so we want to use it.
LIBPJSIP_CONF_OPTS += --enable-epoll

ifeq ($(LINGMO_PACKAGE_ALSA_LIB_MIXER),y)
LIBPJSIP_DEPENDENCIES += alsa-lib
LIBPJSIP_CONF_OPTS += --enable-sound
else
LIBPJSIP_CONF_OPTS += --disable-sound
endif

ifeq ($(LINGMO_PACKAGE_BCG729),y)
LIBPJSIP_DEPENDENCIES += bcg729
LIBPJSIP_CONF_OPTS += --with-bcg729=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-bcg729
endif

ifeq ($(LINGMO_PACKAGE_FFMPEG),y)
LIBPJSIP_DEPENDENCIES += ffmpeg
LIBPJSIP_CONF_OPTS += --with-ffmpeg=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-ffmpeg
endif

ifeq ($(LINGMO_PACKAGE_LIBGSM),y)
LIBPJSIP_CONF_OPTS += \
	--enable-gsm-codec \
	--with-external-gsm
LIBPJSIP_DEPENDENCIES += libgsm
else
LIBPJSIP_CONF_OPTS += \
	--disable-gsm-codec
endif

ifeq ($(LINGMO_PACKAGE_LIBOPENH264),y)
LIBPJSIP_DEPENDENCIES += libopenh264
LIBPJSIP_CONF_OPTS += --with-openh264=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-openh264
endif

ifeq ($(LINGMO_PACKAGE_LIBOPENSSL),y)
LIBPJSIP_DEPENDENCIES += libopenssl
LIBPJSIP_CONF_OPTS += --with-ssl=$(STAGING_DIR)/usr
else ifeq ($(LINGMO_PACKAGE_GNUTLS),y)
LIBPJSIP_DEPENDENCIES += gnutls
LIBPJSIP_CONF_OPTS += --with-gnutls=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-ssl
endif

ifeq ($(LINGMO_PACKAGE_LIBSAMPLERATE),y)
LIBPJSIP_DEPENDENCIES += libsamplerate
LIBPJSIP_CONF_OPTS += --enable-libsamplerate
else
LIBPJSIP_CONF_OPTS += --disable-libsamplerate
endif

ifeq ($(LINGMO_PACKAGE_LIBSRTP),y)
LIBPJSIP_DEPENDENCIES += libsrtp
LIBPJSIP_CONF_OPTS += \
	--enable-libsrtp \
	--with-external-srtp
else
LIBPJSIP_CONF_OPTS += --disable-libsrtp
endif

ifeq ($(LINGMO_PACKAGE_LIBUPNP),y)
LIBPJSIP_DEPENDENCIES += libupnp
LIBPJSIP_CONF_OPTS += --with-upnp=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-upnp
endif

ifeq ($(LINGMO_PACKAGE_LIBV4L),y)
# --enable-v4l2 is broken (check for libv4l2 will be omitted)
LIBPJSIP_DEPENDENCIES += libv4l
else
LIBPJSIP_CONF_OPTS += --disable-v4l2
endif

ifeq ($(LINGMO_PACKAGE_LIBYUV),y)
LIBPJSIP_DEPENDENCIES += libyuv
LIBPJSIP_CONF_OPTS += \
	--enable-libyuv \
	--with-external-yuv
else
LIBPJSIP_CONF_OPTS += --disable-libyuv
endif

ifeq ($(LINGMO_PACKAGE_OPENCORE_AMR),y)
LIBPJSIP_DEPENDENCIES += opencore-amr
LIBPJSIP_CONF_OPTS += --with-opencore-amr=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-opencore-amr
endif

ifeq ($(LINGMO_PACKAGE_OPUS),y)
LIBPJSIP_DEPENDENCIES += opus
LIBPJSIP_CONF_OPTS += --with-opus=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-opus
endif

ifeq ($(LINGMO_PACKAGE_PORTAUDIO),y)
LIBPJSIP_DEPENDENCIES += portaudio
LIBPJSIP_CONF_OPTS += --with-external-pa
else
LIBPJSIP_CONF_OPTS += --without-external-pa
endif

ifeq ($(LINGMO_PACKAGE_SDL2),y)
LIBPJSIP_DEPENDENCIES += sdl2
LIBPJSIP_CONF_OPTS += --with-sdl=$(STAGING_DIR)/usr
else
LIBPJSIP_CONF_OPTS += --disable-sdl
endif

ifeq ($(LINGMO_PACKAGE_SPEEX)$(LINGMO_PACKAGE_SPEEXDSP),yy)
LIBPJSIP_CONF_OPTS += \
	--enable-speex-aec \
	--enable-speex-codec \
	--with-external-speex
LIBPJSIP_DEPENDENCIES += speex speexdsp
else
LIBPJSIP_CONF_OPTS += \
	--disable-speex-aec \
	--disable-speex-codec
endif

ifeq ($(LINGMO_PACKAGE_UTIL_LINUX_LIBUUID),y)
LIBPJSIP_DEPENDENCIES += util-linux
LIBPJSIP_CONF_OPTS += --enable-libuuid
else
LIBPJSIP_CONF_OPTS += --disable-libuuid
endif

# disable build of test binaries
LIBPJSIP_MAKE_OPTS = lib

$(eval $(autotools-package))
