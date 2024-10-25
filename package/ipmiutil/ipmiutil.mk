################################################################################
#
# ipmiutil
#
################################################################################

IPMIUTIL_VERSION = 3.1.9
IPMIUTIL_SITE = https://downloads.sourceforge.net/project/ipmiutil
IPMIUTIL_LICENSE = BSD-3-Clause
IPMIUTIL_LICENSE_FILES = COPYING

IPMIUTIL_MAKE = $(MAKE1)
IPMIUTIL_CONF_ENV = ac_cv_type_wchar_t=$(if $(LINGMO_USE_WCHAR),yes,no)

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
# tests against distro libcrypto so it might get a false positive when
# the openssl version is old, so force it off
# SKIP_MD2 can be used only if ALLOW_GNU is defined.
IPMIUTIL_CONF_OPTS += CPPFLAGS="$(TARGET_CPPFLAGS) -DALLOW_GNU -DSKIP_MD2 -DSSL11"
IPMIUTIL_DEPENDENCIES += openssl
else
IPMIUTIL_CONF_OPTS += --disable-lanplus
endif

$(eval $(autotools-package))
