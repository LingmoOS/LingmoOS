#.rst:
# FindQRencode
# ------------
#
# Try to find the qrencode library.
#
# This will define the following variables:
#
# ``QRencode_FOUND``
#     True if the QRencode library is available.
#
# ``QRencode_INCLUDE_DIRS``
#     the QRencode library include dirs.
#     This variable shall be passed to target_include_libraries() calls if
#     the target is not used for linking.
#
# ``QRencode_LIBRARIES``
#     the libraries used to link QRencode.
#     This can be passed to target_link_libraries instead of
#     the ``QRencode::QRencode`` target.
#
# If ``QRencode_FOUND`` is TRUE, the following imported target
# will be available:
#
# ``QRencode::QRencode``
#     The QRencode library.
#
# Imported target since 5.27.0
#
#=============================================================================
# SPDX-FileCopyrightText: 2010 Sune Vuorela <sune@debian.org>
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_library(QRencode_LIBRARIES NAMES qrencode qrencoded)

find_path(QRencode_INCLUDE_DIRS qrencode.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(QRencode
    FOUND_VAR QRencode_FOUND
    REQUIRED_VARS QRencode_LIBRARIES QRencode_INCLUDE_DIRS
)

if(QRencode_FOUND AND NOT TARGET QRencode::QRencode)
  add_library(QRencode::QRencode UNKNOWN IMPORTED)
  set_target_properties(QRencode::QRencode PROPERTIES
  IMPORTED_LOCATION "${QRencode_LIBRARIES}"
  INTERFACE_INCLUDE_DIRECTORIES "${QRencode_INCLUDE_DIRS}")
endif()

mark_as_advanced(QRencode_LIBRARIES QRencode_INCLUDE_DIRS)

include(FeatureSummary)
set_package_properties(QRencode PROPERTIES
  URL "https://fukuchi.org/works/qrencode/"
  DESCRIPTION "The QRencode library"
)
