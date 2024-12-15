# - Try to find libxcb
# Once done this will define
#
#  LIBXKB_FOUND - system has libxcb
#  LIBXKB_LIBRARIES - Link these to use libxcb-xkb
#  LIBXKB_INCLUDE_DIR - the libxcb-xkb include dir
#  LIBXKB_DEFINITIONS - compiler switches required for using libxcb

# Copyright (c) 2013, Abdurrahman AVCI, <abdurrahmanavci@gmail.com>
# Copyright (c) 2008, Helio Chissini de Castro, <helio@kde.org>
# Copyright (c) 2007, Matthias Kretz, <kretz@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


IF (NOT WIN32)
  IF (LIBXKB_INCLUDE_DIR AND LIBXKB_LIBRARIES)
    # in cache already
    SET(XKB_FIND_QUIETLY TRUE)
  ENDIF (LIBXKB_INCLUDE_DIR AND LIBXKB_LIBRARIES)

  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  FIND_PACKAGE(PkgConfig)
  PKG_CHECK_MODULES(PKG_XKB xcb-xkb)

  SET(LIBXKB_DEFINITIONS ${PKG_XKB_CFLAGS})

  FIND_PATH(LIBXKB_INCLUDE_DIR xcb/xkb.h ${PKG_XKB_INCLUDE_DIRS})

  FIND_LIBRARY(LIBXKB_LIBRARIES NAMES xcb-xkb libxcb-xkb PATHS ${PKG_XKB_LIBRARY_DIRS})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(XKB DEFAULT_MSG LIBXKB_LIBRARIES LIBXKB_INCLUDE_DIR)

  MARK_AS_ADVANCED(LIBXKB_INCLUDE_DIR LIBXKB_LIBRARIES)
ENDIF (NOT WIN32)

#.rst:
# FindXKB
# -------
#
# Try to find xkbcommon on a Unix system
# If found, this will define the following variables:
#
#   ``XKB_FOUND``
#     True if XKB is available
#   ``XKB_LIBRARIES``
#     Link these to use XKB
#   ``XKB_INCLUDE_DIRS``
#     Include directory for XKB
#   ``XKB_DEFINITIONS``
#     Compiler flags for using XKB
#
# Additionally, the following imported targets will be defined:
#
#   ``XKB::XKB``
#     The XKB library

if(NOT WIN32)
    # Use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package(PkgConfig)
    pkg_check_modules(PKG_XKB QUIET xkbcommon)

    set(XKB_DEFINITIONS ${PKG_XKB_CFLAGS_OTHER})

    find_path(XKB_INCLUDE_DIR
        NAMES
            xkbcommon/xkbcommon.h
        HINTS
            ${PKG_XKB_INCLUDE_DIRS}
    )
    find_library(XKB_LIBRARY
        NAMES
            xkbcommon
        HINTS
            ${PKG_XKB_LIBRARY_DIRS}
    )

    set(XKB_LIBRARIES ${XKB_LIBRARY})
    set(XKB_INCLUDE_DIRS ${XKB_INCLUDE_DIR})
    set(XKB_VERSION ${PKG_XKB_VERSION})

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(XKB
        FOUND_VAR
            XKB_FOUND
        REQUIRED_VARS
            XKB_LIBRARY
            XKB_INCLUDE_DIR
        VERSION_VAR
            XKB_VERSION
    )

    if(XKB_FOUND AND NOT TARGET XKB::XKB)
        add_library(XKB::XKB UNKNOWN IMPORTED)
        set_target_properties(XKB::XKB PROPERTIES
            IMPORTED_LOCATION "${XKB_LIBRARY}"
            INTERFACE_COMPILE_OPTIONS "${XKB_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${XKB_INCLUDE_DIR}"
        )
    endif()

else()
    message(STATUS "FindXKB.cmake cannot find XKB on Windows systems.")
    set(XKB_FOUND FALSE)
endif()

include(FeatureSummary)
set_package_properties(XKB PROPERTIES
    URL "https://xkbcommon.org"
    DESCRIPTION "XKB API common to servers and clients"
)