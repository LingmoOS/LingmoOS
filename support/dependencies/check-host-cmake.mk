# The cmake minimum version is set to either 3.18 or higher,
# depending on the highest minimum version required by any
# of the packages bundled in Buildroot. If a package is
# bumped or a new one added, and it requires a higher
# cmake version than the one provided by the host, our
# cmake infra will catch it and build its own.
#
LINGMO_CMAKE_VERSION_MIN = $(LINGMO_HOST_CMAKE_AT_LEAST)

LINGMO_CMAKE_CANDIDATES ?= cmake cmake3
LINGMO_CMAKE ?= $(call suitable-host-package,cmake,\
	$(LINGMO_CMAKE_VERSION_MIN) $(LINGMO_CMAKE_CANDIDATES))
ifeq ($(LINGMO_CMAKE),)
LINGMO_CMAKE = $(HOST_DIR)/bin/cmake
LINGMO_CMAKE_HOST_DEPENDENCY = host-cmake
endif
