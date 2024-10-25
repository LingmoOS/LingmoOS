################################################################################
#
# libopenssl
#
################################################################################

LIBOPENSSL_VERSION = 3.2.2
LIBOPENSSL_SITE = https://www.openssl.org/source
LIBOPENSSL_SOURCE = openssl-$(LIBOPENSSL_VERSION).tar.gz
LIBOPENSSL_LICENSE = Apache-2.0
LIBOPENSSL_LICENSE_FILES = LICENSE.txt
LIBOPENSSL_INSTALL_STAGING = YES
LIBOPENSSL_DEPENDENCIES = zlib
HOST_LIBOPENSSL_DEPENDENCIES = host-zlib
LIBOPENSSL_TARGET_ARCH = $(call qstrip,$(LINGMO_PACKAGE_LIBOPENSSL_TARGET_ARCH))
LIBOPENSSL_CFLAGS = $(TARGET_CFLAGS)
LIBOPENSSL_PROVIDES = openssl
LIBOPENSSL_CPE_ID_VENDOR = $(LIBOPENSSL_PROVIDES)
LIBOPENSSL_CPE_ID_PRODUCT = $(LIBOPENSSL_PROVIDES)

ifeq ($(LINGMO_m68k_cf),y)
# relocation truncated to fit: R_68K_GOT16O
LIBOPENSSL_CFLAGS += -mxgot
# resolves an assembler "out of range error" with blake2 and sha512 algorithms
LIBOPENSSL_CFLAGS += -DOPENSSL_SMALL_FOOTPRINT
endif

ifeq ($(LINGMO_USE_MMU),)
LIBOPENSSL_CFLAGS += -DHAVE_FORK=0 -DHAVE_MADVISE=0
endif

ifeq ($(LINGMO_PACKAGE_CRYPTODEV_LINUX),y)
LIBOPENSSL_DEPENDENCIES += cryptodev-linux
endif

# fixes the following build failures:
#
# - musl
#   ./libcrypto.so: undefined reference to `getcontext'
#   ./libcrypto.so: undefined reference to `setcontext'
#   ./libcrypto.so: undefined reference to `makecontext'
#
# - uclibc:
#   crypto/async/arch/../arch/async_posix.h:32:5: error: unknown type name 'ucontext_t'
#

ifeq ($(LINGMO_TOOLCHAIN_USES_MUSL),y)
LIBOPENSSL_CFLAGS += -DOPENSSL_NO_ASYNC
endif
ifeq ($(LINGMO_TOOLCHAIN_HAS_UCONTEXT),)
LIBOPENSSL_CFLAGS += -DOPENSSL_NO_ASYNC
endif

define HOST_LIBOPENSSL_CONFIGURE_CMDS
	cd $(@D); \
		$(HOST_CONFIGURE_OPTS) \
		./config \
		--prefix=$(HOST_DIR) \
		--openssldir=$(HOST_DIR)/etc/ssl \
		no-docs \
		no-tests \
		no-fuzz-libfuzzer \
		no-fuzz-afl \
		shared \
		zlib-dynamic
endef

define LIBOPENSSL_CONFIGURE_CMDS
	cd $(@D); \
		$(TARGET_CONFIGURE_ARGS) \
		$(TARGET_CONFIGURE_OPTS) \
		 CFLAGS="$(LIBOPENSSL_CFLAGS)" \
		./Configure \
			$(LIBOPENSSL_TARGET_ARCH) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			$(if $(LINGMO_TOOLCHAIN_HAS_THREADS),threads,no-threads) \
			$(if $(LINGMO_STATIC_LIBS),no-shared,shared) \
			$(if $(LINGMO_PACKAGE_CRYPTODEV_LINUX),enable-devcryptoeng) \
			no-rc5 \
			enable-camellia \
			no-docs \
			no-tests \
			no-fuzz-libfuzzer \
			no-fuzz-afl \
			no-afalgeng \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_BIN),,no-apps) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_CHACHA),,no-chacha) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_RC2),,no-rc2) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_RC4),,no-rc4) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_MD2),,no-md2) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_MD4),,no-md4) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_MDC2),,no-mdc2) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_BLAKE2),,no-blake2) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_IDEA),,no-idea) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_SEED),,no-seed) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_DES),,no-des) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_RMD160),,no-rmd160) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_WHIRLPOOL),,no-whirlpool) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_BLOWFISH),,no-bf) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_SSL),,no-ssl) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_SSL3),,no-ssl3) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_WEAK_SSL),,no-weak-ssl-ciphers) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_PSK),,no-psk) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_CAST),,no-cast) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_UNSECURE),,no-unit-test no-crypto-mdebug no-autoerrinit) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_DYNAMIC_ENGINE),,no-dynamic-engine ) \
			$(if $(LINGMO_PACKAGE_LIBOPENSSL_ENABLE_COMP),,no-comp) \
			$(if $(LINGMO_STATIC_LIBS),zlib,zlib-dynamic) \
			$(if $(LINGMO_STATIC_LIBS),no-dso)
endef

# libdl is not available in a static build, and this is not implied by no-dso
ifeq ($(LINGMO_STATIC_LIBS),y)
define LIBOPENSSL_FIXUP_STATIC_MAKEFILE
	$(SED) 's#-ldl##g' $(@D)/Makefile
endef
LIBOPENSSL_POST_CONFIGURE_HOOKS += LIBOPENSSL_FIXUP_STATIC_MAKEFILE
endif

define HOST_LIBOPENSSL_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBOPENSSL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define LIBOPENSSL_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(STAGING_DIR) install
endef

define HOST_LIBOPENSSL_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D) install
endef

define LIBOPENSSL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
	$(RM) -rf $(TARGET_DIR)/usr/lib/ssl
	$(RM) -f $(TARGET_DIR)/usr/bin/c_rehash
endef

# libdl has no business in a static build
ifeq ($(LINGMO_STATIC_LIBS),y)
define LIBOPENSSL_FIXUP_STATIC_PKGCONFIG
	$(SED) 's#-ldl##' $(STAGING_DIR)/usr/lib/pkgconfig/libcrypto.pc
	$(SED) 's#-ldl##' $(STAGING_DIR)/usr/lib/pkgconfig/libssl.pc
	$(SED) 's#-ldl##' $(STAGING_DIR)/usr/lib/pkgconfig/openssl.pc
endef
LIBOPENSSL_POST_INSTALL_STAGING_HOOKS += LIBOPENSSL_FIXUP_STATIC_PKGCONFIG
endif

ifeq ($(LINGMO_PACKAGE_PERL),)
define LIBOPENSSL_REMOVE_PERL_SCRIPTS
	$(RM) -f $(TARGET_DIR)/etc/ssl/misc/{CA.pl,tsget}
endef
LIBOPENSSL_POST_INSTALL_TARGET_HOOKS += LIBOPENSSL_REMOVE_PERL_SCRIPTS
endif

ifeq ($(LINGMO_PACKAGE_LIBOPENSSL_ENGINES),)
define LIBOPENSSL_REMOVE_LIBOPENSSL_ENGINES
	$(RM) -rf $(TARGET_DIR)/usr/lib/engines-3
endef
LIBOPENSSL_POST_INSTALL_TARGET_HOOKS += LIBOPENSSL_REMOVE_LIBOPENSSL_ENGINES
endif

$(eval $(generic-package))
$(eval $(host-generic-package))
