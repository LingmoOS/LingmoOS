# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

find_program(coredumpctl_EXE coredumpctl)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(coredumpctl
    FOUND_VAR
        coredumpctl_FOUND
    REQUIRED_VARS
        coredumpctl_EXE
)
