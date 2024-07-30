#=============================================================================
# SPDX-FileCopyrightText: 2019 Daniel Vrátil <dvratil@kde.org>
# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#=============================================================================

# On Debian systems the daemon can be inside various lib directories...
set(boltd_PATHS
    /usr/lib/bolt/ # 0.5+ on Debian and Ubuntu
)
if(CMAKE_LIBRARY_ARCHITECTURE)
    list(APPEND boltd_PATHS
        # 0.5 on Ubuntu
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/
    )
endif()

find_program(boltd_EXECUTABLE
    NAMES boltd
    PATHS ${boltd_PATHS}
    PATH_SUFFIXES bin libexec
)
find_package_handle_standard_args(bolt
    FOUND_VAR
        bolt_FOUND
    REQUIRED_VARS
        boltd_EXECUTABLE
)
mark_as_advanced(boltd_EXECUTABLE)
