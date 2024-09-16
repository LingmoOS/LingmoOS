# Copyright (c) 2012 CSSlayer <wengxt@gmail.com>
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

if(XKBFILE_INCLUDE_DIR AND XKBFILE_LIBRARIES)
    # Already in cache, be silent
    set(XKBFILE_FIND_QUIETLY TRUE)
endif(XKBFILE_INCLUDE_DIR AND XKBFILE_LIBRARIES)

find_package(PkgConfig)
pkg_check_modules(PC_LIBXKBFILE xkbfile)

find_path(XKBFILE_MAIN_INCLUDE_DIR
          NAMES XKBfile.h
          HINTS ${PC_LIBXKBFILE_INCLUDEDIR}
          PATH_SUFFIXES "X11/extensions")

find_library(XKBFILE_LIBRARIES
             NAMES xkbfile
             HINTS ${PC_LIBXKBFILE_LIBDIR})

set(XKBFILE_INCLUDE_DIR "${XKBFILE_MAIN_INCLUDE_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XkbFile  DEFAULT_MSG  XKBFILE_LIBRARIES XKBFILE_MAIN_INCLUDE_DIR)

mark_as_advanced(XKBFILE_INCLUDE_DIR XKBFILE_LIBRARIES)
