# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

find_program(smartctl_EXE smartctl)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(smartctl
    FOUND_VAR
        smartctl_FOUND
    REQUIRED_VARS
        smartctl_EXE
)
