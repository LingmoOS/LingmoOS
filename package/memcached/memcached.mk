################################################################################
#
# memcached
#
################################################################################

MEMCACHED_VERSION = 1.6.22
MEMCACHED_SITE = http://www.memcached.org/files
MEMCACHED_DEPENDENCIES = libevent
MEMCACHED_CONF_ENV = ac_cv_prog_cc_c99='-std=gnu99'
MEMCACHED_CONF_OPTS = --disable-coverage --disable-werror
MEMCACHED_LICENSE = BSD-3-Clause
MEMCACHED_LICENSE_FILES = COPYING
MEMCACHED_CPE_ID_VENDOR = memcached
MEMCACHED_SELINUX_MODULES = memcached
# We're patching configure.ac
MEMCACHED_AUTORECONF = YES

ifeq ($(LINGMO_ENDIAN),"BIG")
MEMCACHED_CONF_ENV += ac_cv_c_endian=big
else
MEMCACHED_CONF_ENV += ac_cv_c_endian=little
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
MEMCACHED_CONF_OPTS += --enable-tls
MEMCACHED_DEPENDENCIES += host-pkgconf openssl
else
MEMCACHED_CONF_OPTS += --disable-tls
endif

ifeq ($(LINGMO_STATIC_LIBS),)
MEMCACHED_CONF_OPTS += --disable-static
endif

$(eval $(autotools-package))
