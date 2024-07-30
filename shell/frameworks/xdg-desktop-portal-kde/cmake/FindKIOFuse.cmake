# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

execute_process(
    COMMAND dbus-send --session --print-reply --dest=org.freedesktop.DBus  /org/freedesktop/DBus org.freedesktop.DBus.ListActivatableNames
    OUTPUT_VARIABLE _kiofuseOut)

set(${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
if("${_kiofuseOut}" MATCHES "\"org.kde.KIOFuse\"")
    set(${CMAKE_FIND_PACKAGE_NAME}_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${CMAKE_FIND_PACKAGE_NAME}
    FOUND_VAR ${CMAKE_FIND_PACKAGE_NAME}_FOUND
    REQUIRED_VARS ${CMAKE_FIND_PACKAGE_NAME}_FOUND
    REASON_FAILURE_MESSAGE "Could not find DBus service org.kde.KIOFuse in org.freedesktop.DBus.ListActivatableNames"
)
