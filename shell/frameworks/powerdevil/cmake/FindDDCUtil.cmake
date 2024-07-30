# - Try to find Libddcutil
# Once done this will define
#
#  DDCUTIL_FOUND - system has DDCUtil
#  DDCUTIL_INCLUDE_DIR - the libddcutil include directory
#  DDCUTIL_LIBS - The libddcutil libraries

# SPDX-FileCopyrightText: 2017 Dorian Vogel <dorianvogel@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(PkgConfig)
pkg_check_modules(PC_LIBDDCUTIL QUIET ddcutil)
set(LIBDDCUTIL_DEFINITIONS ${PC_LIBDDCUTIL_CFLAGS_OTHER})

find_path(LIBDDCUTIL_INCLUDE_DIR ddcutil_c_api.h
          HINTS ${PC_LIBDDCUTIL_INCLUDEDIR} ${PC_LIBDDCUTIL_INCLUDE_DIRS})

find_library(LIBDDCUTIL_LIBRARY NAMES libddcutil.so
             HINTS ${PC_LIBDDCUTIL_LIBDIR} ${PC_LIBDDCUTIL_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBDDCUTIL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(DDCUtil DEFAULT_MSG
                                  LIBDDCUTIL_LIBRARY LIBDDCUTIL_INCLUDE_DIR)

mark_as_advanced(LIBDDCUTIL_INCLUDE_DIR LIBDDCUTIL_LIBRARY )

set(LIBDDCUTIL_LIBRARIES ${LIBDDCUTIL_LIBRARY} )
set(LIBDDCUTIL_INCLUDE_DIRS ${LIBDDCUTIL_INCLUDE_DIR} )
