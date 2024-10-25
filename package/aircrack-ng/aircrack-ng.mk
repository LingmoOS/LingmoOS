################################################################################
#
# aircrack-ng
#
################################################################################

AIRCRACK_NG_VERSION = 1.7
AIRCRACK_NG_SITE = http://download.aircrack-ng.org
AIRCRACK_NG_LICENSE = GPL-2.0+
AIRCRACK_NG_LICENSE_FILES = LICENSE
AIRCRACK_NG_CPE_ID_VENDOR = aircrack-ng
AIRCRACK_NG_DEPENDENCIES = \
	$(if $(LINGMO_PACKAGE_CMOCKA),cmocka) \
	$(if $(LINGMO_PACKAGE_LIBNL),libnl) \
	$(if $(LINGMO_PACKAGE_OPENSSL),openssl) \
	$(if $(LINGMO_PACKAGE_PCRE),pcre) \
	$(if $(LINGMO_PACKAGE_ZLIB),zlib) \
	host-pkgconf
AIRCRACK_NG_AUTORECONF = YES
# Enable buddy-ng, easside-ng, tkiptun-ng, wesside-ng
AIRCRACK_NG_CONF_OPTS = --with-experimental

ifeq ($(LINGMO_TOOLCHAIN_HAS_SSP),y)
AIRCRACK_NG_CONF_OPTS += --with-opt
else
AIRCRACK_NG_CONF_OPTS += --without-opt
endif

ifeq ($(LINGMO_PACKAGE_DUMA),y)
AIRCRACK_NG_DEPENDENCIES += duma
AIRCRACK_NG_CONF_OPTS += --with-duma
else
AIRCRACK_NG_CONF_OPTS += --without-duma
endif

ifeq ($(LINGMO_PACKAGE_HWLOC),y)
AIRCRACK_NG_DEPENDENCIES += hwloc
AIRCRACK_NG_CONF_OPTS += --enable-hwloc
else
AIRCRACK_NG_CONF_OPTS += --disable-hwloc
endif

ifeq ($(LINGMO_PACKAGE_JEMALLOC),y)
AIRCRACK_NG_DEPENDENCIES += jemalloc
AIRCRACK_NG_CONF_OPTS += --with-jemalloc
else
AIRCRACK_NG_CONF_OPTS += --without-jemalloc
endif

ifeq ($(LINGMO_PACKAGE_LIBGCRYPT),y)
AIRCRACK_NG_DEPENDENCIES += libgcrypt
AIRCRACK_NG_CONF_OPTS += \
	--with-gcrypt \
	--with-libgcrypt-prefix=$(STAGING_DIR)/usr
else
AIRCRACK_NG_CONF_OPTS += --without-gcrypt
endif

ifeq ($(LINGMO_PACKAGE_LIBPCAP),y)
AIRCRACK_NG_DEPENDENCIES += libpcap
AIRCRACK_NG_CONF_OPTS += \
	--with-libpcap-include=$(STAGING_DIR)/usr/include \
	--with-libpcap-lib=$(STAGING_DIR)/usr/lib
ifeq ($(LINGMO_STATIC_LIBS),y)
AIRCRACK_NG_CONF_ENV += \
	LIBS="`$(STAGING_DIR)/usr/bin/pcap-config --static --additional-libs`"
endif
endif

ifeq ($(LINGMO_PACKAGE_SQLITE),y)
AIRCRACK_NG_DEPENDENCIES += sqlite
AIRCRACK_NG_CONF_OPTS += --with-sqlite3=$(STAGING_DIR)/usr
else
AIRCRACK_NG_CONF_OPTS += --without-sqlite3
endif

$(eval $(autotools-package))
