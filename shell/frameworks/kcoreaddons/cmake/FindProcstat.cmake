# SPDX-FileCopyrightText: 2019 Tobias C. Berner <tcberner@FreeBSD.org>
#
# SPDX-License-Identifier: BSD-2-Clause
#[=======================================================================[.rst:
FindProcstat
-------

Finds the procstat library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Procstat::Procstat``
  The procstat library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Procstat_FOUND``
  True if the system has the libprocstat library.
``Procstat_INCLUDE_DIRS``
  Include directories needed to use procstat.
``Procstat_LIBRARIES``
  Libraries needed to link to libprocstat.

#]=======================================================================]


find_path(Procstat_INCLUDE_DIRS libprocstat.h)

find_library(Procstat_LIBRARIES NAMES procstat)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Procstat DEFAULT_MSG Procstat_INCLUDE_DIRS Procstat_LIBRARIES)

set_package_properties(Procstat PROPERTIES
    URL "https://github.com/freebsd/freebsd/tree/master/lib/libprocstat"
    DESCRIPTION "Library to access process information"
)
mark_as_advanced(Procstat_INCLUDE_DIRS Procstat_LIBRARIES)
if(Procstat_FOUND AND NOT TARGET Procstat::Procstat)
    add_library(Procstat::Procstat UNKNOWN IMPORTED)
    set_target_properties(Procstat::Procstat PROPERTIES
        IMPORTED_LOCATION "${Procstat_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${Procstat_INCLUDE_DIRS}"
    )
endif()

