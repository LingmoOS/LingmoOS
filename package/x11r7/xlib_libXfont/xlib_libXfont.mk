################################################################################
#
# xlib_libXfont
#
################################################################################

XLIB_LIBXFONT_VERSION = 1.5.4
XLIB_LIBXFONT_SOURCE = libXfont-$(XLIB_LIBXFONT_VERSION).tar.bz2
XLIB_LIBXFONT_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXFONT_LICENSE = MIT
XLIB_LIBXFONT_LICENSE_FILES = COPYING
XLIB_LIBXFONT_INSTALL_STAGING = YES

XLIB_LIBXFONT_DEPENDENCIES = freetype xlib_libfontenc xlib_xtrans xorgproto

HOST_XLIB_LIBXFONT_DEPENDENCIES = \
	host-freetype host-xlib_libfontenc host-xlib_xtrans \
	host-xorgproto

XLIB_LIBXFONT_CONF_OPTS = --disable-devel-docs
HOST_XLIB_LIBXFONT_CONF_OPTS = --disable-devel-docs

ifeq ($(LINGMO_microblaze),y)
# The microblaze toolchains don't define the __ELF__ preprocessor
# symbol even though they do use the elf format. LibXfont checks for
# this symbol to know if weak symbols are supported, and otherwise
# falls back to emulation code using dlopen - Causing linker issues
# for stuff using libXfont.
# Work around it by defining the symbol here as well.
XLIB_LIBXFONT_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -D__ELF__"
endif

XLIB_LIBXFONT_CFLAGS = $(TARGET_CFLAGS)

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_85180),y)
XLIB_LIBXFONT_CFLAGS += -O0
endif

XLIB_LIBXFONT_CONF_ENV += CFLAGS="$(XLIB_LIBXFONT_CFLAGS)"

$(eval $(autotools-package))
$(eval $(host-autotools-package))
