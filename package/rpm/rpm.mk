################################################################################
#
# rpm
#
################################################################################

RPM_VERSION_MAJOR = 4.18
RPM_VERSION = $(RPM_VERSION_MAJOR).1
RPM_SOURCE = rpm-$(RPM_VERSION).tar.bz2
RPM_SITE = http://ftp.rpm.org/releases/rpm-$(RPM_VERSION_MAJOR).x
RPM_DEPENDENCIES = \
	host-pkgconf \
	$(if $(LINGMO_PACKAGE_BZIP2),bzip2) \
	$(if $(LINGMO_PACKAGE_ELFUTILS),elfutils) \
	file \
	lua \
	popt \
	$(if $(LINGMO_PACKAGE_XZ),xz) \
	zlib \
	$(TARGET_NLS_DEPENDENCIES)
RPM_LICENSE = GPL-2.0 or LGPL-2.0 (library only)
RPM_LICENSE_FILES = COPYING
RPM_CPE_ID_VENDOR = rpm
RPM_SELINUX_MODULES = rpm

# Don't set --{dis,en}-openmp as upstream wants to abort the build if
# --enable-openmp is provided and OpenMP is < 4.5:
# https://github.com/rpm-software-management/rpm/pull/1433
RPM_CONF_OPTS = \
	--disable-rpath \
	--with-gnu-ld \
	--without-fapolicyd \
	--without-fsverity \
	--without-imaevm

ifeq ($(LINGMO_PACKAGE_ACL),y)
RPM_DEPENDENCIES += acl
RPM_CONF_OPTS += --with-acl
else
RPM_CONF_OPTS += --without-acl
endif

ifeq ($(LINGMO_PACKAGE_AUDIT),y)
RPM_DEPENDENCIES += audit
RPM_CONF_OPTS += --with-audit
else
RPM_CONF_OPTS += --without-audit
endif

ifeq ($(LINGMO_PACKAGE_DBUS),y)
RPM_DEPENDENCIES += dbus
RPM_CONF_OPTS += --enable-plugins
else
RPM_CONF_OPTS += --disable-plugins
endif

ifeq ($(LINGMO_PACKAGE_LIBCAP),y)
RPM_DEPENDENCIES += libcap
RPM_CONF_OPTS += --with-cap
else
RPM_CONF_OPTS += --without-cap
endif

ifeq ($(LINGMO_PACKAGE_LIBGCRYPT),y)
RPM_DEPENDENCIES += libgcrypt
RPM_CONF_OPTS += --with-crypto=libgcrypt
else
RPM_DEPENDENCIES += openssl
RPM_CONF_OPTS += --with-crypto=openssl
endif

ifeq ($(LINGMO_PACKAGE_GETTEXT_PROVIDES_LIBINTL),y)
RPM_CONF_OPTS += --with-libintl-prefix=$(STAGING_DIR)/usr
else
RPM_CONF_OPTS += --without-libintl-prefix
endif

ifeq ($(LINGMO_PACKAGE_LIBSELINUX),y)
RPM_DEPENDENCIES += libselinux
RPM_CONF_OPTS += --with-selinux
else
RPM_CONF_OPTS += --without-selinux
endif

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
RPM_DEPENDENCIES += python3
RPM_CONF_OPTS += --enable-python
else
RPM_CONF_OPTS += --disable-python
endif

ifeq ($(LINGMO_PACKAGE_READLINE),y)
RPM_DEPENDENCIES += readline
RPM_CONF_OPTS += --with-readline
else
RPM_CONF_OPTS += --without-readline
endif

ifeq ($(LINGMO_PACKAGE_SQLITE),y)
RPM_DEPENDENCIES += sqlite
RPM_CONF_OPTS += --enable-sqlite
else
RPM_CONF_OPTS += --disable-sqlite
endif

ifeq ($(LINGMO_PACKAGE_ZSTD),y)
RPM_DEPENDENCIES += zstd
RPM_CONF_OPTS += --enable-zstd
else
RPM_CONF_OPTS += --disable-zstd
endif

ifeq ($(LINGMO_PACKAGE_RPM_RPM2ARCHIVE),y)
RPM_DEPENDENCIES += libarchive
RPM_CONF_OPTS += --with-archive
else
RPM_CONF_OPTS += --without-archive
endif

# ac_cv_prog_cc_c99: RPM uses non-standard GCC extensions (ex. `asm`).
RPM_CONF_ENV = \
	ac_cv_prog_cc_c99='-std=gnu99' \
	LIBS=$(TARGET_NLS_LIBS)

$(eval $(autotools-package))
