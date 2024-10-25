################################################################################
#
# libtalloc
#
################################################################################

LIBTALLOC_VERSION = 2.4.2
LIBTALLOC_SOURCE = talloc-$(LIBTALLOC_VERSION).tar.gz
LIBTALLOC_SITE = https://www.samba.org/ftp/talloc
LIBTALLOC_LICENSE = LGPL-3.0+
LIBTALLOC_LICENSE_FILES = talloc.h
LIBTALLOC_INSTALL_STAGING = YES

# libtalloc is extracted from the samba source tree, and that has a workaround
# that requires PYTHONHASHSEED to be set, and to be set to 1.
# See https://gitlab.com/samba-team/samba/-/commit/420bbb1d92fd2a28725b53f425ba3d214831b660
LIBTALLOC_CONF_ENV = PYTHONHASHSEED=1
LIBTALLOC_MAKE_ENV = PYTHONHASHSEED=1

# --with-libiconv= is unconditionally passed, even if libiconv is not
# present. Indeed, waf will search for libiconv by default in
# /usr/local. Because of a bug in some waf python script, /usr/local
# is then used in many subsequent and unrelated checks, which
# ultimately causes a failure when LINGMO_COMPILER_PARANOID_UNSAFE_PATH
# is set.  However no need to set libiconv as a dependency of
# libtalloc since it's optional.
LIBTALLOC_CONF_OPTS += --cross-compile \
		--cross-answers=$(@D)/cache.txt \
		--disable-stack-protector \
		--hostcc=gcc \
		--with-libiconv=$(STAGING_DIR)/usr

ifeq ($(LINGMO_PACKAGE_LIBTIRPC),y)
LIBTALLOC_DEPENDENCIES += libtirpc host-pkgconf
endif

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
LIBTALLOC_DEPENDENCIES += host-python3 python3
LIBTALLOC_CONF_ENV += \
	PYTHON="$(HOST_DIR)/bin/python3" \
	PYTHON_CONFIG="$(STAGING_DIR)/usr/bin/python3-config"
# There isn't any --enable-python configuration option
else
LIBTALLOC_CONF_OPTS += --disable-python
endif

LIBTALLOC_WAF = ./buildtools/bin/waf

# like samba4, libtalloc uses the waf build system which requires a
# proper answers file to configure package before build
define LIBTALLOC_POPULATE_WAF_CACHE
	$(INSTALL) -m 0644 package/libtalloc/libtalloc-cache.txt $(@D)/cache.txt
	echo 'Checking uname machine type: $(LINGMO_ARCH)' >> $(@D)/cache.txt
endef

LIBTALLOC_PRE_CONFIGURE_HOOKS += LIBTALLOC_POPULATE_WAF_CACHE

$(eval $(waf-package))
