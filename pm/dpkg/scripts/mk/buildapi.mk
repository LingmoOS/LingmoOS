# This Makefile fragment (since dpkg 1.22.0) defines the DPKG_BUILD_API
# variable, that contains the package dpkg-build-api(7) level.
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_buildapi_mk_included
dpkg_buildapi_mk_included = yes

# Default API level when not set.
DPKG_BUILD_API ?= $(shell dpkg-buildapi)

# We could use only built-in GNU make functions, but that seems too much
# complexity given no integer operators, given that we currently have to
# fetch the build API level anyway.
dpkg_build_api_ge = $(shell test "$(DPKG_BUILD_API)" -ge "$(1)" && echo yes)

endif # dpkg_buildapi_mk_included
