#.rst:
# FindLibIntl
# ---------
#
# Find libintl
#
# Find the libintl headers and libraries. On platforms that use glibc this is not required
# and LibIntl_LIBRARIES will be empty
#
# ::
#
#   LibIntl_INCLUDE_DIRS - where to find libintl.h
#   LibIntl_LIBRARIES    - The libintl library if the current platform does not use glibc.
#   LibIntl_FOUND        - True if libintl was found.

#=============================================================================
# SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

find_path(LibIntl_INCLUDE_DIRS NAMES libintl.h)
find_library(LibIntl_LIBRARIES NAMES intl libintl)

include(CheckCXXSymbolExists)
include(CMakePushCheckState)
check_cxx_symbol_exists(dngettext libintl.h LibIntl_SYMBOL_FOUND)

include(FindPackageHandleStandardArgs)
if(LibIntl_SYMBOL_FOUND)
    message(STATUS "libintl is part of libc, no extra library is required.")
    set(LibIntl_LIBRARIES "")
    if(LibIntl_INCLUDE_DIRS)
        find_package_handle_standard_args(LibIntl REQUIRED_VARS LibIntl_INCLUDE_DIRS)
    else()
        # in the default search path but not found by find_path, e.g. host glibc when cross-compiling
        set(LibIntl_INCLUDE_DIRS "")
        set(LibIntl_FOUND TRUE)
    endif()
else()
    message(STATUS "libintl is a separate library.")
    find_package_handle_standard_args(LibIntl REQUIRED_VARS LibIntl_INCLUDE_DIRS LibIntl_LIBRARIES)
endif()

# make sure we have -Wl,--no-undefined here, otherwise this test will always pass
cmake_push_check_state()
set(CMAKE_REQUIRED_LIBRARIES ${LibIntl_LIBRARIES} ${CMAKE_SHARED_LINKER_FLAGS})
check_cxx_source_compiles("extern \"C\" int _nl_msg_cat_cntr; int main(void) { ++_nl_msg_cat_cntr; return 0; }" HAVE_NL_MSG_CAT_CNTR)
cmake_pop_check_state()
