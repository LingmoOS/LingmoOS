#.rst:
# FindPList
# --------
#
# Try to find the plist library, once done this will define:
#
#  ``PList_FOUND``
#      System has libplist.
#
#  ``PList_INCLUDE_DIRS``
#      The libplist include directory.
#
# ``PList_LIBRARIES``
#     The libplist libraries.
#
# ``PList_VERSION``
#     The libplist version.
#
# If ``PList_FOUND`` is TRUE, the following imported target
# will be available:
#
# ``PList::PList``
#     The libplist library

#=============================================================================
# SPDX-FileCopyrightText: 2020 MBition GmbH
# SPDX-FileContributor: Kai Uwe Broulik <kai_uwe.broulik@mbition.io>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig QUIET)
pkg_search_module(PC_libplist QUIET libplist-2.0 libplist)

find_path(PList_INCLUDE_DIRS NAMES plist/plist.h HINTS ${PC_libplist_INCLUDE_DIRS})
find_library(PList_LIBRARIES NAMES plist-2.0 plist HINTS ${PC_libplist_LIBRARY_DIRS})

set(PList_VERSION ${PC_libplist_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PList
    FOUND_VAR PList_FOUND
    REQUIRED_VARS PList_INCLUDE_DIRS PList_LIBRARIES
    VERSION_VAR PList_VERSION
)

mark_as_advanced(PList_INCLUDE_DIRS PList_LIBRARIES)

if(PList_FOUND AND NOT TARGET PList::PList)
    add_library(PList::PList UNKNOWN IMPORTED)
    set_target_properties(PList::PList PROPERTIES
        IMPORTED_LOCATION "${PList_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${PList_INCLUDE_DIRS}"
        INTERFACE_COMPILE_OPTIONS "${PC_libplist_CFLAGS_OTHER}"
    )
endif()

include(FeatureSummary)
set_package_properties(PList PROPERTIES
    DESCRIPTION "library to handle Apple property list format"
    URL "https://www.libimobiledevice.org/"
)
