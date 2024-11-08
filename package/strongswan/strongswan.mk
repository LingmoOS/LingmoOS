################################################################################
#
# strongswan
#
################################################################################

STRONGSWAN_VERSION = 5.9.13
STRONGSWAN_SOURCE = strongswan-$(STRONGSWAN_VERSION).tar.bz2
STRONGSWAN_SITE = http://download.strongswan.org
STRONGSWAN_LICENSE = GPL-2.0+
STRONGSWAN_LICENSE_FILES = COPYING LICENSE
STRONGSWAN_CPE_ID_VENDOR = strongswan
STRONGSWAN_DEPENDENCIES = host-pkgconf
STRONGSWAN_INSTALL_STAGING = YES
STRONGSWAN_CONF_OPTS += \
	--without-lib-prefix \
	--enable-led \
	--enable-pkcs11=yes \
	--enable-kernel-netlink=yes \
	--enable-socket-default=yes \
	--enable-botan=$(if $(LINGMO_PACKAGE_STRONGSWAN_BOTAN),yes,no) \
	--enable-openssl=$(if $(LINGMO_PACKAGE_STRONGSWAN_OPENSSL),yes,no) \
	--enable-gcrypt=$(if $(LINGMO_PACKAGE_STRONGSWAN_GCRYPT),yes,no) \
	--enable-gmp=$(if $(LINGMO_PACKAGE_STRONGSWAN_GMP),yes,no) \
	--enable-af-alg=$(if $(LINGMO_PACKAGE_STRONGSWAN_AF_ALG),yes,no) \
	--enable-curl=$(if $(LINGMO_PACKAGE_STRONGSWAN_CURL),yes,no) \
	--enable-charon=$(if $(LINGMO_PACKAGE_STRONGSWAN_CHARON),yes,no) \
	--enable-tnccs-11=$(if $(LINGMO_PACKAGE_STRONGSWAN_TNCCS_11),yes,no) \
	--enable-tnccs-20=$(if $(LINGMO_PACKAGE_STRONGSWAN_TNCCS_20),yes,no) \
	--enable-tnccs-dynamic=$(if $(LINGMO_PACKAGE_STRONGSWAN_TNCCS_DYNAMIC),yes,no) \
	--enable-eap-sim-pcsc=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIM_PCSC),yes,no) \
	--enable-unity=$(if $(LINGMO_PACKAGE_STRONGSWAN_UNITY),yes,no) \
	--enable-stroke=$(if $(LINGMO_PACKAGE_STRONGSWAN_STROKE),yes,no) \
	--enable-sql=$(if $(LINGMO_PACKAGE_STRONGSWAN_SQL),yes,no) \
	--enable-pki=$(if $(LINGMO_PACKAGE_STRONGSWAN_PKI),yes,no) \
	--enable-scripts=$(if $(LINGMO_PACKAGE_STRONGSWAN_SCRIPTS),yes,no) \
	--enable-vici=$(if $(LINGMO_PACKAGE_STRONGSWAN_VICI),yes,no) \
	--enable-swanctl=$(if $(LINGMO_PACKAGE_STRONGSWAN_VICI),yes,no) \
	--enable-wolfssl=$(if $(LINGMO_PACKAGE_STRONGSWAN_WOLFSSL),yes,no) \
	--enable-md4=$(if $(LINGMO_PACKAGE_STRONGSWAN_MD4),yes,no) \
	--enable-systime-fix=$(if $(LINGMO_PACKAGE_STRONGSWAN_SYSTIME_FIX),yes,no) \
	--enable-eap-sim=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIM),yes,no) \
	--enable-eap-sim-file=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIM_FILE),yes,no) \
	--enable-eap-aka=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_AKA),yes,no) \
	--enable-eap-aka-3gpp2=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_AKA_3GPP2),yes,no) \
	--enable-eap-simaka-sql=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIMAKA_SQL),yes,no) \
	--enable-eap-simaka-pseudonym=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIMAKA_PSEUDONYM),yes,no) \
	--enable-eap-simaka-reauth=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIMAKA_REAUTH),yes,no) \
	--enable-eap-identity=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_IDENTITY),yes,no) \
	--enable-eap-md5=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_MD5),yes,no) \
	--enable-eap-gtc=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_GTC),yes,no) \
	--enable-eap-mschapv2=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_MSCHAPV2),yes,no) \
	--enable-eap-tls=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_TLS),yes,no) \
	--enable-eap-ttls=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_TTLS),yes,no) \
	--enable-eap-peap=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_PEAP),yes,no) \
	--enable-eap-tnc=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_TNC),yes,no) \
	--enable-eap-dynamic=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_DYNAMIC),yes,no) \
	--enable-eap-radius=$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_RADIUS),yes,no) \
	--enable-bypass-lan=$(if $(LINGMO_PACKAGE_STRONGSWAN_BYPASS_LAN),yes,no) \
	--with-ipseclibdir=/usr/lib \
	--with-plugindir=/usr/lib/ipsec/plugins \
	--with-imcvdir=/usr/lib/ipsec/imcvs \
	--with-dev-headers=/usr/include

ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
STRONGSWAN_CONF_ENV += LIBS='-latomic'
endif

STRONGSWAN_DEPENDENCIES += \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_BOTAN),botan) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_OPENSSL),openssl) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_GCRYPT),libgcrypt) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_GMP),gmp) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_AKA_3GPP2),gmp) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_CURL),libcurl) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_TNCCS_11),libxml2) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_EAP_SIM_PCSC),pcsc-lite) \
	$(if $(LINGMO_PACKAGE_STRONGSWAN_WOLFSSL),wolfssl)

ifeq ($(LINGMO_PACKAGE_STRONGSWAN_SQL),y)
STRONGSWAN_DEPENDENCIES += \
	$(if $(LINGMO_PACKAGE_SQLITE),sqlite) \
	$(if $(LINGMO_PACKAGE_MARIADB),mariadb)
endif

# disable connmark/forecast until net/if.h vs. linux/if.h conflict resolved
# problem exist since linux 4.5 header changes
STRONGSWAN_CONF_OPTS += \
	--disable-connmark \
	--disable-forecast

$(eval $(autotools-package))
