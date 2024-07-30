#.rst:
# FindLibIcal
# -----------
#
# Try to find the Ical libraries.
#
# This will define the following variables:
#
# ``LibIcal_FOUND``
#     True if a suitable LibIcal was found
#  ``LibIcal_INCLUDE_DIRS``
#     This should be passed to target_include_directories() if
#     the target is not used for linking
#  ``LibIcal_LIBRARIES``
#     The Ical libraries (ical + icalss)
#     This can be passed to target_link_libraries() instead of
#     the ``LibIcal`` target
#
# ``LibIcal_VERSION``
#     The LibIcal version defined in ical.h
# If ``LibIcal_FOUND`` is TRUE, the following imported target
# will be available:
#
# ``LibIcal``
#     The Ical libraries
#
# The following variables are set for compatibility reason and will be
# removed in the next major version
#  ``LibIcal_MAJOR_VERSION``
#     The LibIcal major version
#  ``LibIcal_MINOR_VERSION``
#     The LibIcal minor version
#
# SPDX-FileCopyrightText: 2008, 2010 Allen Winter <winter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

if(NOT LibIcal_FIND_VERSION)
  set(LibIcal_FIND_VERSION "0.33")
endif()

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LibIcal QUIET libical)

find_path(LibIcal_INCLUDE_DIRS
  NAMES libical/ical.h
  HINTS ${PC_LibIcal_INCLUDEDIR}
)

find_library(LibIcal_LIBRARY
  NAMES ical libical
  HINTS ${PC_LibIcal_LIBDIR}
)

find_library(LibIcalss_LIBRARY
  NAMES icalss libicalss
  HINTS ${PC_LibIcal_LIBDIR}
)

find_library(LibIcalvcal_LIBRARY
  NAMES icalvcal
  HINTS ${PC_LibIcal_LIBDIR}
)

# For backward compatibility
set(LibIcal_INCLUDE_DIRS "${LibIcal_INCLUDE_DIRS}" "${LibIcal_INCLUDE_DIRS}/libical")

set(LibIcal_LIBRARIES ${LibIcal_LIBRARY} ${LibIcalss_LIBRARY} ${LibIcalvcal_LIBRARY})

set(LibIcal_VERSION "${PC_LibIcal_VERSION}")

if(NOT ICAL_H)
  find_file(ICAL_H ical.h HINTS ${LibIcal_INCLUDE_DIRS})
endif()

if(NOT LibIcal_VERSION)
  if(EXISTS "${ICAL_H}")
    file(STRINGS "${ICAL_H}" _ICAL_H_VERSION REGEX "^#define[ ]+ICAL_VERSION[ ]+\"[0-9].[0-9]\"$")
    string(REGEX REPLACE "^#define[ ]+ICAL_VERSION[ ]+\"([0-9].[0-9])\"$" "\\1" LibIcal_VERSION "${_ICAL_H_VERSION}")
    file(STRINGS "${ICAL_H}" _ICAL_H_PATCH_VERSION REGEX "^#define[ ]+ICAL_PATCH_VERSION[ ]+\\([0-9]+\\)$")
    string(REGEX REPLACE "^#define[ ]+ICAL_PATCH_VERSION[ ]+\\(([0-9]+)\\)$" "\\1" LibIcal_PATCH_VERSION "${_ICAL_H_PATCH_VERSION}")
    unset(_ICAL_H_VERSION)
    unset(_ICAL_H_PATCH_VERSION)
    if(LibIcal_PATCH_VERSION)
      set(LibIcal_VERSION "${LibIcal_VERSION}.${LibIcal_PATCH_VERSION}")
    endif()
  endif()
endif()

# For compatibility
string(REGEX REPLACE "^([0-9]).[0-9]$" "\\1" LibIcal_MAJOR_VERSION "${LibIcal_VERSION}")
string(REGEX REPLACE "^[0-9].([0-9])$" "\\1" LibIcal_MINOR_VERSION "${LibIcal_VERSION}")

if(NOT LibIcal_VERSION VERSION_LESS 0.46)
  set(USE_ICAL_0_46 TRUE)
endif()
if(NOT LibIcal_VERSION VERSION_LESS 1.00)
  set(USE_ICAL_1_0 TRUE)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LibIcal
    FOUND_VAR LibIcal_FOUND
    REQUIRED_VARS LibIcal_LIBRARIES LibIcal_INCLUDE_DIRS
    VERSION_VAR LibIcal_VERSION
)

# Internal
if(LibIcal_FOUND AND NOT TARGET LibIcalss)
  add_library(LibIcalss UNKNOWN IMPORTED)
  set_target_properties(LibIcalss PROPERTIES
  IMPORTED_LOCATION "${LibIcalss_LIBRARY}")
endif()
if(LibIcal_FOUND AND NOT TARGET LibIcalvcal)
  add_library(LibIcalvcal UNKNOWN IMPORTED)
  set_target_properties(LibIcalvcal PROPERTIES
  IMPORTED_LOCATION "${LibIcalvcal_LIBRARY}")
endif()

# Public Target
if(LibIcal_FOUND AND NOT TARGET LibIcal)
  add_library(LibIcal UNKNOWN IMPORTED)
  set_target_properties(LibIcal PROPERTIES
    IMPORTED_LOCATION "${LibIcal_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${LibIcal_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "LibIcalss;LibIcalvcal"
  )
endif()

mark_as_advanced(LibIcal_INCLUDE_DIRS LibIcal_LIBRARY LibIcalss_LIBRARY LibIcalvcal_LIBRARY LibIcal_LIBRARIES)

include(FeatureSummary)
set_package_properties(LibIcal PROPERTIES
  URL "https://github.com/libical/libical"
  DESCRIPTION "Implementation of iCalendar protocols and data formats"
)
