################################################################################
#
# osm2pgsql
#
################################################################################

OSM2PGSQL_VERSION = 1.10.0
OSM2PGSQL_SITE = $(call github,openstreetmap,osm2pgsql,$(OSM2PGSQL_VERSION))
OSM2PGSQL_LICENSE = GPL-2.0+
OSM2PGSQL_LICENSE_FILES = COPYING
OSM2PGSQL_SUPPORTS_IN_SOURCE_BUILD = NO

OSM2PGSQL_DEPENDENCIES = boost bzip2 expat json-for-modern-cpp libosmium postgresql protozero zlib

# fmt > 8.0 is not yet supported
OSM2PGSQL_CONF_OPTS = \
	-DBUILD_TESTS=OFF \
	-DBUILD_COVERAGE=OFF \
	-DEXTERNAL_FMT=OFF \
	-DEXTERNAL_LIBOSMIUM=ON \
	-DEXTERNAL_PROTOZERO=ON

ifeq ($(LINGMO_PACKAGE_LUAJIT),y)
OSM2PGSQL_DEPENDENCIES += luajit
OSM2PGSQL_CONF_OPTS += -DWITH_LUA=ON -DWITH_LUAJIT=ON
else ifeq ($(LINGMO_PACKAGE_LUA),y)
OSM2PGSQL_DEPENDENCIES += lua
OSM2PGSQL_CONF_OPTS += -DWITH_LUA=ON -DWITH_LUAJIT=OFF
else
OSM2PGSQL_CONF_OPTS += -DWITH_LUA=OFF -DWITH_LUAJIT=OFF
endif

ifeq ($(LINGMO_PACKAGE_PROJ),y)
OSM2PGSQL_DEPENDENCIES += proj
OSM2PGSQL_CONF_OPTS += -DUSE_PROJ_LIB=auto
else
OSM2PGSQL_CONF_OPTS += -DUSE_PROJ_LIB=off
endif

$(eval $(cmake-package))
