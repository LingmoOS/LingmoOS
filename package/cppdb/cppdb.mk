################################################################################
#
# cppdb
#
################################################################################

CPPDB_VERSION = 0.3.1
CPPDB_SOURCE = cppdb-$(CPPDB_VERSION).tar.bz2
CPPDB_SITE = http://downloads.sourceforge.net/project/cppcms/cppdb/$(CPPDB_VERSION)
CPPDB_INSTALL_STAGING = YES
CPPDB_DEPENDENCIES = $(if $(LINGMO_PACKAGE_SQLITE),sqlite)
CPPDB_LICENSE = BSL-1.0 or MIT
CPPDB_LICENSE_FILES = LICENSE_1_0.txt MIT.txt

ifeq ($(LINGMO_PACKAGE_MARIADB),y)
CPPDB_DEPENDENCIES += mariadb
else
CPPDB_CONF_OPTS += -DDISABLE_MYSQL=ON
endif

ifeq ($(LINGMO_PACKAGE_POSTGRESQL),y)
CPPDB_DEPENDENCIES += postgresql
else
CPPDB_CONF_OPTS += -DDISABLE_PQ=ON
endif

ifeq ($(LINGMO_PACKAGE_SQLITE),)
CPPDB_CONF_OPTS += -DDISABLE_SQLITE=ON
endif

$(eval $(cmake-package))
