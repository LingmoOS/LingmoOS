#=============================================================================
# SPDX-FileCopyrightText: 2017 Tobias C. Berner <tcberner@FreeBSD.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================
find_program(cklistsessions_EXECUTABLE NAMES ck-list-sessions)
find_program(qdbus_EXECUTABLE NAMES qdbus)
find_package_handle_standard_args(ConsoleKit
    FOUND_VAR
        ConsoleKit_FOUND
    REQUIRED_VARS
        cklistsessions_EXECUTABLE
        qdbus_EXECUTABLE
)
mark_as_advanced(ConsoleKit_FOUND cklistsessions_EXECUTABLE qdbus_EXECUTABLE)
