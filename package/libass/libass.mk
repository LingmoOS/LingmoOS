################################################################################
#
# libass
#
################################################################################

LIBASS_VERSION = 0.17.1
LIBASS_SOURCE = libass-$(LIBASS_VERSION).tar.xz
# Do not use the github helper here, the generated tarball is *NOT*
# the same as the one uploaded by upstream for the release.
LIBASS_SITE = https://github.com/libass/libass/releases/download/$(LIBASS_VERSION)
LIBASS_INSTALL_STAGING = YES
LIBASS_LICENSE = ISC
LIBASS_LICENSE_FILES = COPYING
LIBASS_CPE_ID_VALID = YES
LIBASS_DEPENDENCIES = \
	host-pkgconf \
	freetype \
	harfbuzz \
	libfribidi \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv)

# configure: WARNING: Install nasm for a significantly faster libass build.
# only for Intel archs
ifeq ($(LINGMO_i386)$(LINGMO_x86_64),y)
LIBASS_DEPENDENCIES += host-nasm
endif

ifeq ($(LINGMO_PACKAGE_FONTCONFIG),y)
LIBASS_DEPENDENCIES += fontconfig
LIBASS_CONF_OPTS += --enable-fontconfig
else
LIBASS_CONF_OPTS += --disable-fontconfig --disable-require-system-font-provider
endif

ifeq ($(LINGMO_PACKAGE_LIBUNIBREAK),y)
LIBASS_DEPENDENCIES += libunibreak
LIBASS_CONF_OPTS += --enable-libunibreak
else
LIBASS_CONF_OPTS += --disable-libunibreak
endif

$(eval $(autotools-package))
