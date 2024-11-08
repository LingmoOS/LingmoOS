################################################################################
#
# gettext-gnu
#
################################################################################

# Please keep in sync with GETTEXT_TINY_ARCHIVE_VERSION in
# gettext-tiny/gettext-tiny.mk
GETTEXT_GNU_VERSION = 0.22.4
GETTEXT_GNU_SITE = $(LINGMO_GNU_MIRROR)/gettext
GETTEXT_GNU_SOURCE = gettext-$(GETTEXT_GNU_VERSION).tar.xz
GETTEXT_GNU_INSTALL_STAGING = YES
GETTEXT_GNU_LICENSE = LGPL-2.1+ (libintl), GPL-3.0+ (the rest)
GETTEXT_GNU_LICENSE_FILES = COPYING gettext-runtime/intl/COPYING.LIB
GETTEXT_GNU_CPE_ID_VENDOR = gnu
GETTEXT_GNU_CPE_ID_PRODUCT = gettext
GETTEXT_GNU_PROVIDES = gettext
GETTEXT_GNU_DEPENDENCIES = $(if $(LINGMO_PACKAGE_LIBICONV),libiconv)

# Avoid using the bundled subset of libxml2
HOST_GETTEXT_GNU_DEPENDENCIES = host-libxml2

GETTEXT_GNU_CONF_OPTS += \
	--disable-libasprintf \
	--disable-acl \
	--disable-openmp \
	--disable-rpath \
	--disable-java \
	--disable-native-java \
	--disable-csharp \
	--disable-relocatable \
	--without-emacs

HOST_GETTEXT_GNU_CONF_OPTS = \
	--disable-libasprintf \
	--disable-acl \
	--disable-openmp \
	--disable-rpath \
	--disable-java \
	--disable-native-java \
	--disable-csharp \
	--disable-relocatable \
	--without-emacs

# Force the build of libintl, even if the C library provides a stub
# gettext implementation
ifeq ($(LINGMO_PACKAGE_GETTEXT_PROVIDES_LIBINTL),y)
GETTEXT_GNU_CONF_OPTS += --with-included-gettext
else
GETTEXT_GNU_CONF_OPTS += --without-included-gettext
endif

# For the target version, we only need the runtime.
GETTEXT_GNU_SUBDIR = gettext-runtime
# For the host variant, we only need the tools, but those need the
# runtime, so it is much simpler to build the whole package. _SUBDIR
# for the host is inherited from the target if not set or empty, so
# we need to explicitly set it to build the whole package.
HOST_GETTEXT_GNU_SUBDIR = .

# Disable the build of documentation and examples of gettext-tools,
# and the build of documentation and tests of gettext-runtime.
define HOST_GETTEXT_GNU_DISABLE_UNNEEDED
	$(SED) '/^SUBDIRS/s/ doc //;/^SUBDIRS/s/examples$$//' $(@D)/gettext-tools/Makefile.in
	$(SED) '/^SUBDIRS/s/ doc //;/^SUBDIRS/s/tests$$//' $(@D)/gettext-runtime/Makefile.in
endef

GETTEXT_GNU_POST_PATCH_HOOKS += HOST_GETTEXT_GNU_DISABLE_UNNEEDED
HOST_GETTEXT_GNU_POST_PATCH_HOOKS += HOST_GETTEXT_GNU_DISABLE_UNNEEDED

define GETTEXT_GNU_REMOVE_UNNEEDED
	$(RM) -rf $(TARGET_DIR)/usr/share/gettext/ABOUT-NLS
	rmdir --ignore-fail-on-non-empty $(TARGET_DIR)/usr/share/gettext
endef

GETTEXT_GNU_POST_INSTALL_TARGET_HOOKS += GETTEXT_GNU_REMOVE_UNNEEDED

# Force build with NLS support, otherwise libintl is not built
# This is needed because some packages (eg. libglib2) requires
# locales, but do not properly depend on LINGMO_ENABLE_LOCALE, and
# instead select LINGMO_PACKAGE_GETTEXT_GNU. Those packages need to be
# fixed before we can remove the following 3 lines... :-(
ifeq ($(LINGMO_ENABLE_LOCALE),)
GETTEXT_GNU_CONF_OPTS += --enable-nls
endif

# Disable interactive confirmation in host gettextize for package fixups
define HOST_GETTEXT_GNU_GETTEXTIZE_CONFIRMATION
	$(SED) '/read dummy/d' $(HOST_DIR)/bin/gettextize
endef
HOST_GETTEXT_GNU_POST_INSTALL_HOOKS += HOST_GETTEXT_GNU_GETTEXTIZE_CONFIRMATION

$(eval $(autotools-package))
$(eval $(host-autotools-package))
