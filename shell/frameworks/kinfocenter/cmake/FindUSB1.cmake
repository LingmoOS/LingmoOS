# - Try to find libusb-1.0
# Once done this will define
#
#  USB1_FOUND - system has libusb-1.0
#  USB1_INCLUDE_DIRS - the libusb-1.0 include directories
#  USB1_LIBRARIES - Link these to use libusb-1.0
#  USB1_DEFINITIONS - Compiler switches required for using libusb-1.0

#=============================================================================
# SPDX-FileCopyrightText: 2017 Pino Toscano <toscano.pino@tiscali.it>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_package(PkgConfig)
pkg_check_modules(PC_LIBUSB1 QUIET libusb-1.0)

find_path(USB1_INCLUDE_DIR
  NAMES
    libusb.h
  HINTS
    ${PC_LIBUSB1_INCLUDE_DIRS}
  PATH_SUFFIXES
    libusb-1.0
)

find_library(USB1_LIBRARY
  NAMES
    ${PC_LIBUSB1_LIBRARIES}
    usb-1.0
  HINTS
    ${PC_LIBUSB1_LIBRARY_DIRS}
)

set(USB1_INCLUDE_DIRS ${USB1_INCLUDE_DIR})
set(USB1_LIBRARIES ${USB1_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(USB1
  FOUND_VAR
    USB1_FOUND
  REQUIRED_VARS
    USB1_LIBRARY
    USB1_INCLUDE_DIR
  VERSION_VAR
    PC_LIBUSB1_VERSION
)
if(USB1_FOUND AND NOT TARGET Libusb-1.0)
  add_library(Libusb-1.0 SHARED IMPORTED)
  set_target_properties(Libusb-1.0 PROPERTIES
    IMPORTED_LOCATION "${USB1_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${USB1_INCLUDE_DIRS}"
  )
endif()

mark_as_advanced(USB1_INCLUDE_DIRS USB1_LIBRARIES)
