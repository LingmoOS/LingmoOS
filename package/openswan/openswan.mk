################################################################################
#
# openswan
#
################################################################################

OPENSWAN_VERSION = 3.0.0
OPENSWAN_SITE = $(call github,xelerance,Openswan,v$(OPENSWAN_VERSION))
OPENSWAN_LICENSE = GPL-2.0+, BSD-3-Clause
OPENSWAN_LICENSE_FILES = COPYING LICENSE
OPENSWAN_CPE_ID_VENDOR = xelerance

OPENSWAN_DEPENDENCIES = host-bison host-flex gmp iproute2
OPENSWAN_MAKE_OPTS = ARCH=$(LINGMO_ARCH) CC="$(TARGET_CC)" POD2MAN="" XMLTO="" \
	USERCOMPILE="$(TARGET_CFLAGS) $(if $(LINGMO_TOOLCHAIN_SUPPORTS_PIE),-fPIE)" \
	USERLINK="$(TARGET_LDFLAGS) $(if $(LINGMO_TOOLCHAIN_SUPPORTS_PIE),-fPIE)" \
	INC_USRLOCAL=/usr USE_KLIPS=false USE_MAST=false USE_NM=false \
	USE_NOMANINSTALL=true WERROR=""

ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
OPENSWAN_DEPENDENCIES += libcurl
OPENSWAN_MAKE_OPTS += USE_LIBCURL=true
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
OPENSWAN_DEPENDENCIES += openssl
OPENSWAN_MAKE_OPTS += HAVE_OPENSSL=true
endif

define OPENSWAN_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) \
		$(OPENSWAN_MAKE_OPTS) programs
endef

define OPENSWAN_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) \
		$(OPENSWAN_MAKE_OPTS) DESTDIR=$(TARGET_DIR) install
endef

$(eval $(generic-package))
