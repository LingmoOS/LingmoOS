################################################################################
#
# netsurf
#
################################################################################

NETSURF_VERSION = 3.10
NETSURF_SOURCE = netsurf-all-$(NETSURF_VERSION).tar.gz
NETSURF_SITE = http://download.netsurf-browser.org/netsurf/releases/source-full
NETSURF_LICENSE = GPL-2.0
NETSURF_LICENSE_FILES = netsurf/COPYING
NETSURF_CPE_ID_VENDOR = netsurf-browser
# host-vim needed for the xxd utility
NETSURF_DEPENDENCIES = expat jpeg libpng \
	host-bison host-flex host-gperf host-pkgconf host-vim

# internal duktape doesn't build with LINGMO_OPTIMIZE_FAST
ifeq ($(LINGMO_OPTIMIZE_FAST),y)
define NETSURF_DUKTAPE_CONFIGURE_CMDS
	echo "override NETSURF_USE_DUKTAPE := NO"       >> $(@D)/netsurf/Makefile.config
endef
else
define NETSURF_DUKTAPE_CONFIGURE_CMDS
	echo "override NETSURF_USE_DUKTAPE := YES"      >> $(@D)/netsurf/Makefile.config
endef
endif

ifeq ($(LINGMO_PACKAGE_NETSURF_GTK),y)
NETSURF_DEPENDENCIES += libgtk2
NETSURF_FRONTEND = gtk2
endif

ifeq ($(LINGMO_PACKAGE_NETSURF_GTK3),y)
NETSURF_DEPENDENCIES += libgtk3
NETSURF_FRONTEND = gtk3
endif

ifeq ($(LINGMO_PACKAGE_NETSURF_GTK)$(LINGMO_PACKAGE_NETSURF_GTK3),y)
ifeq ($(LINGMO_PACKAGE_LIBRSVG),y)
NETSURF_DEPENDENCIES += librsvg
define NETSURF_SVG_CONFIGURE_CMDS
	echo "override NETSURF_USE_RSVG := YES"         >> $(@D)/netsurf/Makefile.config
	echo "override NETSURF_USE_NSSVG := NO"         >> $(@D)/netsurf/Makefile.config
endef
endif
endif

ifeq ($(LINGMO_PACKAGE_NETSURF_SDL),y)
NETSURF_DEPENDENCIES += sdl host-libpng
NETSURF_FRONTEND = framebuffer
NETSURF_CONFIG = \
	BUILD_CFLAGS='$(HOST_CFLAGS)' \
	BUILD_LDFLAGS='$(HOST_LDFLAGS) -lpng'
ifeq ($(LINGMO_PACKAGE_FREETYPE),y)
NETSURF_DEPENDENCIES += freetype
define NETSURF_FONTLIB_CONFIGURE_CMDS
	echo "override NETSURF_FB_FONTLIB := freetype"  >> $(@D)/netsurf/Makefile.config
endef
endif
endif

ifeq ($(LINGMO_PACKAGE_LIBICONV),y)
NETSURF_DEPENDENCIES += libiconv
define NETSURF_ICONV_CONFIGURE_CMDS
	echo "CFLAGS += -DWITH_ICONV_FILTER"            >> $(@D)/libparserutils/Makefile.config.override
	echo "override NETSURF_USE_LIBICONV_PLUG := NO" >> $(@D)/netsurf/Makefile.config
endef
endif

ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
NETSURF_DEPENDENCIES += libcurl openssl
else
define NETSURF_CURL_CONFIGURE_CMDS
	echo "override NETSURF_USE_CURL := NO"          >> $(@D)/netsurf/Makefile.config
	echo "override NETSURF_USE_OPENSSL := NO"       >> $(@D)/netsurf/Makefile.config
endef
endif

ifeq ($(LINGMO_PACKAGE_WEBP),y)
NETSURF_DEPENDENCIES += webp
define NETSURF_WEBP_CONFIGURE_CMDS
	echo "override NETSURF_USE_WEBP := YES"         >> $(@D)/netsurf/Makefile.config
endef
else
define NETSURF_WEBP_CONFIGURE_CMDS
	echo "override NETSURF_USE_WEBP := NO"          >> $(@D)/netsurf/Makefile.config
endef
endif

define NETSURF_CONFIGURE_CMDS
	$(NETSURF_DUKTAPE_CONFIGURE_CMDS)
	$(NETSURF_ICONV_CONFIGURE_CMDS)
	$(NETSURF_SVG_CONFIGURE_CMDS)
	$(NETSURF_FONTLIB_CONFIGURE_CMDS)
	$(NETSURF_CURL_CONFIGURE_CMDS)
	$(NETSURF_WEBP_CONFIGURE_CMDS)
endef

NETSURF_MAKE_ENV = \
	$(TARGET_MAKE_ENV) \
	CFLAGS="$(TARGET_CFLAGS) -I$(@D)/tmpusr/include" \
	LDFLAGS="$(TARGET_LDFLAGS) -L$(@D)/tmpusr/lib"

NETSURF_MAKE_OPTS = \
	TARGET=$(NETSURF_FRONTEND) \
	BISON="$(HOST_DIR)/bin/bison" \
	FLEX="$(HOST_DIR)/bin/flex" \
	PKG_CONFIG="$(PKG_CONFIG_HOST_BINARY)" \
	BUILD_CC="$(HOSTCC)" \
	CC="$(TARGET_CC)" \
	AR="$(TARGET_AR)" \
	TMP_PREFIX=$(@D)/tmpusr \
	NETSURF_CONFIG="$(NETSURF_CONFIG)" \
	PREFIX=/usr

define NETSURF_BUILD_CMDS
	mkdir -p $(@D)/tmpusr
	$(NETSURF_MAKE_ENV) $(MAKE) -C $(@D) $(NETSURF_MAKE_OPTS) \
		build
endef

define NETSURF_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) $(NETSURF_MAKE_OPTS) \
		DESTDIR=$(TARGET_DIR) install
endef

$(eval $(generic-package))
