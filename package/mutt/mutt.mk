################################################################################
#
# mutt
#
################################################################################

MUTT_VERSION = 2.2.13
MUTT_SITE = https://bitbucket.org/mutt/mutt/downloads
MUTT_LICENSE = GPL-2.0+
MUTT_LICENSE_FILES = GPL
MUTT_CPE_ID_VENDOR = mutt
MUTT_DEPENDENCIES = ncurses
MUTT_CONF_OPTS = --disable-doc --disable-smtp

ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
MUTT_CONF_ENV += LIBS=-latomic
endif

ifeq ($(LINGMO_PACKAGE_LIBICONV),y)
MUTT_DEPENDENCIES += libiconv
MUTT_CONF_OPTS += --enable-iconv
endif

# Both options can't be selected at the same time so prefer libidn2
ifeq ($(LINGMO_PACKAGE_LIBIDN2),y)
MUTT_DEPENDENCIES += libidn2
MUTT_CONF_OPTS += --with-idn2 --without-idn
else ifeq ($(LINGMO_PACKAGE_LIBIDN),y)
MUTT_DEPENDENCIES += libidn
MUTT_CONF_OPTS += --with-idn --without-idn2
else
MUTT_CONF_OPTS += --without-idn --without-idn2
endif

ifeq ($(LINGMO_PACKAGE_LIBGPGME),y)
MUTT_DEPENDENCIES += libgpgme
MUTT_CONF_OPTS += \
	--enable-gpgme \
	--with-gpgme-prefix=$(STAGING_DIR)/usr

# Force the path to "gpgrt-config" (from the libgpg-error package) to
# avoid using the one on host, if present.
MUTT_GPGRT_CONFIG = $(STAGING_DIR)/usr/bin/gpgrt-config
ifeq ($(LINGMO_STATIC_LIBS),y)
MUTT_GPGRT_CONFIG += --static
endif
MUTT_CONF_ENV += GPGRT_CONFIG="$(MUTT_GPGRT_CONFIG)"
else
MUTT_CONF_OPTS += --disable-gpgme
endif

ifeq ($(LINGMO_PACKAGE_MUTT_IMAP),y)
MUTT_CONF_OPTS += --enable-imap
else
MUTT_CONF_OPTS += --disable-imap
endif

ifeq ($(LINGMO_PACKAGE_MUTT_POP3),y)
MUTT_CONF_OPTS += --enable-pop
else
MUTT_CONF_OPTS += --disable-pop
endif

# SASL and SSL support are only used by imap or pop3 module
ifneq ($(LINGMO_PACKAGE_MUTT_IMAP)$(LINGMO_PACKAGE_MUTT_POP3),)
ifeq ($(LINGMO_PACKAGE_LIBGSASL),y)
MUTT_DEPENDENCIES += libgsasl
MUTT_CONF_OPTS += --with-gsasl
else
MUTT_CONF_OPTS += --without-gsasl
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
MUTT_DEPENDENCIES += openssl
MUTT_CONF_OPTS += \
	--without-gnutls \
	--with-ssl=$(STAGING_DIR)/usr
else ifeq ($(LINGMO_PACKAGE_GNUTLS),y)
MUTT_DEPENDENCIES += gnutls
MUTT_CONF_OPTS += \
	--with-gnutls=$(STAGING_DIR)/usr \
	--without-ssl
else
MUTT_CONF_OPTS += \
	--without-gnutls \
	--without-ssl
endif
else
MUTT_CONF_OPTS += \
	--without-gsasl \
	--without-gnutls \
	--without-ssl
endif

ifeq ($(LINGMO_PACKAGE_SQLITE),y)
MUTT_DEPENDENCIES += sqlite
MUTT_CONF_OPTS += --with-sqlite3
else
MUTT_CONF_OPTS += --without-sqlite3
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
MUTT_DEPENDENCIES += zlib
MUTT_CONF_OPTS += --with-zlib=$(STAGING_DIR)/usr
else
MUTT_CONF_OPTS += --without-zlib
endif

# Avoid running tests to check for:
#  - target system is *BSD
#  - C99 conformance (snprintf, vsnprintf)
#  - behaviour of the regex library
#  - if mail spool directory is world/group writable
#  - we have a working libiconv
MUTT_CONF_ENV += \
	mutt_cv_bsdish=no \
	mutt_cv_c99_snprintf=yes \
	mutt_cv_c99_vsnprintf=yes \
	mutt_cv_regex_broken=no \
	mutt_cv_worldwrite=yes \
	mutt_cv_groupwrite=yes \
	mutt_cv_iconv_good=yes \
	mutt_cv_iconv_nontrans=no

MUTT_CONF_OPTS += --with-mailpath=/var/mail

define MUTT_VAR_MAIL
	mkdir -p $(TARGET_DIR)/var
	ln -sf /tmp $(TARGET_DIR)/var/mail
endef
MUTT_POST_INSTALL_TARGET_HOOKS += MUTT_VAR_MAIL

$(eval $(autotools-package))
