# - Try to find the UTEMPTER directory notification library
# Once done this will define
#
#  UTEMPTER_FOUND - system has UTEMPTER
#  UTEMPTER_INCLUDE_DIR - the UTEMPTER include directory
#  UTEMPTER_LIBRARIES - The libraries needed to use UTEMPTER

# SPDX-FileCopyrightText: 2015 Hrvoje Senjan <hrvoje.senjan@gmail.org>
#
# SPDX-License-Identifier: BSD-3-Clause

if (CMAKE_CROSSCOMPILING)
    # Avoid encoding a host path in the target library
    if (NOT UTEMPTER_EXECUTABLE)
        message(FATAL_ERROR "You must set UTEMPTER_EXECUTABLE or CMAKE_DISABLE_FIND_PACKAGE_UTEMPTER when cross-compiling")
    endif()
else ()
    # use find_file instead of find_program until this is fixed:
    # https://gitlab.kitware.com/cmake/cmake/issues/10468
    find_file (UTEMPTER_EXECUTABLE utempter PATHS
        ${KDE_INSTALL_FULL_LIBEXECDIR}/utempter
        ${KDE_INSTALL_FULL_LIBDIR}/utempter
        ${CMAKE_INSTALL_PREFIX}/libexec/utempter
        ${CMAKE_INSTALL_PREFIX}/lib/utempter
        /usr/libexec/utempter
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/utempter
        /usr/lib/utempter
    )

    # On FreeBSD for example we have to use ulog-helper
    if (NOT UTEMPTER_EXECUTABLE)
        find_program (UTEMPTER_EXECUTABLE ulog-helper PATHS /usr/libexec)
    endif ()
endif ()

if (UTEMPTER_EXECUTABLE)
    set(UTEMPTER_COMPILE_FLAGS -DUTEMPTER_PATH=\"${UTEMPTER_EXECUTABLE}\")
    get_filename_component(UTEMPTER_EXECUTABLE_NAME "${UTEMPTER_EXECUTABLE}" NAME)
    if ("${UTEMPTER_EXECUTABLE_NAME}" STREQUAL ulog-helper)
        message(STATUS "UTEMPTER: using ulog-helper")
        set(UTEMPTER_COMPILE_FLAGS ${UTEMPTER_COMPILE_FLAGS} -DUTEMPTER_ULOG=1)
    endif ()
endif ()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (UTEMPTER REQUIRED_VARS UTEMPTER_EXECUTABLE UTEMPTER_COMPILE_FLAGS)


set_package_properties (UTEMPTER PROPERTIES
    URL "ftp://ftp.altlinux.org/pub/people/ldv/utempter/"
    DESCRIPTION "Allows non-privileged applications such as terminal emulators to modify the utmp database without having to be setuid root."
)

mark_as_advanced (UTEMPTER_EXECUTABLE)

