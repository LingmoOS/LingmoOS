#.rst:
# FindDmtx
# --------
#
# Try to find the DataMatrix library.
#
# This will define the following variables:
#
# ``Dmtx_FOUND``
#     True if the datamatrix library is available.
#
# ``Dmtx_INCLUDE_DIRS``
#     the datamatrix library include dirs.
#     This variable shall be passed to target_include_libraries() calls if
#     the target is not used for linking.
#
# ``Dmtx_LIBRARIES``
#     the libraries used to link datamatrix.
#     This can be passed to target_link_libraries instead of
#     the ``Dmtx::Dmtx`` target.
#
# If ``Dmtx_FOUND`` is TRUE, the following imported target
# will be available:
#
# ``Dmtx::Dmtx``
#     The datamatrix library.
#
# Imported target since 5.27.0
#
#=============================================================================
# SPDX-FileCopyrightText: 2010 Sune Vuorela <sune@debian.org>
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_path(Dmtx_INCLUDE_DIRS dmtx.h)

find_library(Dmtx_LIBRARIES NAMES dmtx)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Dmtx
    FOUND_VAR Dmtx_FOUND
    REQUIRED_VARS Dmtx_LIBRARIES Dmtx_INCLUDE_DIRS
)

if(Dmtx_FOUND AND NOT TARGET Dmtx::Dmtx)
  add_library(Dmtx::Dmtx UNKNOWN IMPORTED)
  set_target_properties(Dmtx::Dmtx PROPERTIES
  IMPORTED_LOCATION "${Dmtx_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${Dmtx_INCLUDE_DIRS}")
endif()

mark_as_advanced(Dmtx_INCLUDE_DIRS Dmtx_LIBRARIES)

include(FeatureSummary)
set_package_properties(Dmtx PROPERTIES
  URL "https://github.com/dmtx/libdmtx"
  DESCRIPTION "The Datamatrix library"
)

