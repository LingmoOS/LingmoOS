# This Makefile fragment (since dpkg 1.20.1) parses option arguments from
# DEB_BUILD_OPTIONS, and exposes these as variables.
#
# Defines the following variables:
#
#   DEB_BUILD_OPTION_PARALLEL: The argument for the parallel=N option.
#     $(DEB_BUILD_OPTIONS)                "parallel=2"  "parallel="  ""
#     $(DEB_BUILD_OPTION_PARALLEL)        "2"           ""           unset
#     $(DEB_BUILD_OPTION_PARALLEL:%=-j%)  "-j2"         ""           ""
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_buildopts_mk_included
dpkg_buildopts_mk_included = yes

dpkg_buildopts_parallel := $(filter parallel=%,$(DEB_BUILD_OPTIONS))
ifdef dpkg_buildopts_parallel
  DEB_BUILD_OPTION_PARALLEL = $(patsubst parallel=%,%,$(dpkg_buildopts_parallel))
endif

endif # dpkg_buildopts_mk_included
