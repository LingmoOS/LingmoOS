#=============================================================================
# SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================
find_program(loginctl_EXECUTABLE NAMES loginctl)
find_package_handle_standard_args(loginctl
    FOUND_VAR
        loginctl_FOUND
    REQUIRED_VARS
        loginctl_EXECUTABLE
)
mark_as_advanced(loginctl_FOUND loginctl_EXECUTABLE)
