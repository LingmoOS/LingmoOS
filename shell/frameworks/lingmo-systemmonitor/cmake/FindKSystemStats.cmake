# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

find_program(ksystemstats_EXE ksystemstats)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ksystemstats
    FOUND_VAR ksystemstats_FOUND
    REQUIRED_VARS ksystemstats_EXE
)
