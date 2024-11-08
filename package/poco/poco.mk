################################################################################
#
# poco
#
################################################################################

POCO_VERSION = 1.13.2
POCO_SITE = $(call github,pocoproject,poco,poco-$(POCO_VERSION)-release)
POCO_LICENSE = BSL-1.0
POCO_LICENSE_FILES = LICENSE
POCO_CPE_ID_VENDOR = pocoproject
POCO_INSTALL_STAGING = YES

POCO_DEPENDENCIES = \
	pcre2 \
	zlib \
	$(if $(LINGMO_PACKAGE_POCO_CRYPTO),openssl) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_MYSQL),mariadb) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_SQLITE),sqlite) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_PGSQL),postgresql) \
	$(if $(LINGMO_PACKAGE_POCO_NETSSL_OPENSSL),openssl) \
	$(if $(LINGMO_PACKAGE_POCO_XML),expat)

POCO_OMIT = \
	Data/ODBC \
	PageCompiler \
	$(if $(LINGMO_PACKAGE_POCO_ACTIVERECORD),,ActiveRecord) \
	$(if $(LINGMO_PACKAGE_POCO_CPP_PARSER),,CppParser) \
	$(if $(LINGMO_PACKAGE_POCO_CRYPTO),,Crypto) \
	$(if $(LINGMO_PACKAGE_POCO_DATA),,Data) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_MYSQL),,Data/MySQL) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_SQLITE),,Data/SQLite) \
	$(if $(LINGMO_PACKAGE_POCO_DATA_PGSQL),,Data/PostgreSQL) \
	$(if $(LINGMO_PACKAGE_POCO_JSON),,JSON) \
	$(if $(LINGMO_PACKAGE_POCO_JWT),,JWT) \
	$(if $(LINGMO_PACKAGE_POCO_MONGODB),,MongoDB) \
	$(if $(LINGMO_PACKAGE_POCO_NET),,Net) \
	$(if $(LINGMO_PACKAGE_POCO_NETSSL_OPENSSL),,NetSSL_OpenSSL) \
	$(if $(LINGMO_PACKAGE_POCO_PDF),,PDF) \
	$(if $(LINGMO_PACKAGE_POCO_PROMETHEUS),,Prometheus) \
	$(if $(LINGMO_PACKAGE_POCO_REDIS),,Redis) \
	$(if $(LINGMO_PACKAGE_POCO_UTIL),,Util) \
	$(if $(LINGMO_PACKAGE_POCO_XML),,XML) \
	$(if $(LINGMO_PACKAGE_POCO_ZIP),,Zip)

ifeq ($(LINGMO_TOOLCHAIN_USES_UCLIBC),y)
POCO_CONF_OPTS += --no-fpenvironment --no-wstring
endif

# architectures missing some FE_* in their fenv.h
ifeq ($(LINGMO_sh4a)$(LINGMO_nios2),y)
POCO_CONF_OPTS += --no-fpenvironment
endif

# disable fpenvironment for soft floating point configuration
ifeq ($(LINGMO_SOFT_FLOAT),y)
POCO_CONF_OPTS += --no-fpenvironment
endif

POCO_MAKE_TARGET = shared_release

POCO_LDFLAGS=$(TARGET_LDFLAGS)
ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
POCO_LDFLAGS += -latomic
endif

define POCO_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) ./configure \
		--config=Linux \
		--prefix=/usr \
		--ldflags="$(POCO_LDFLAGS)" \
		--omit="$(POCO_OMIT)" \
		$(POCO_CONF_OPTS) \
		--unbundled \
		--no-tests \
		--no-samples)
endef

# Use $(MAKE1) to avoid failures on heavilly parallel machines (e.g. -j25)
define POCO_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) POCO_TARGET_OSARCH=$(ARCH) CROSS_COMPILE=$(TARGET_CROSS) \
		POCO_MYSQL_INCLUDE=$(STAGING_DIR)/usr/include/mysql \
		POCO_MYSQL_LIB=$(STAGING_DIR)/usr/lib/mysql \
		POCO_PGSQL_INCLUDE=$(STAGING_DIR)/usr/include/postgresql \
		POCO_PGSQL_LIB=$(STAGING_DIR)/usr/lib/postgresql \
		DEFAULT_TARGET=$(POCO_MAKE_TARGET) -C $(@D)
endef

define POCO_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(STAGING_DIR) POCO_TARGET_OSARCH=$(ARCH) \
		DEFAULT_TARGET=$(POCO_MAKE_TARGET) install -C $(@D)
endef

define POCO_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) DESTDIR=$(TARGET_DIR) POCO_TARGET_OSARCH=$(ARCH) \
		DEFAULT_TARGET=$(POCO_MAKE_TARGET) install -C $(@D)
endef

$(eval $(generic-package))
