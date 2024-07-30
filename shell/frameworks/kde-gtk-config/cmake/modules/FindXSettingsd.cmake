# - Find XSettingsd
# This module defines the following variables:
#
#  XSettingsd_FOUND - true if found
#  XSettingsd_PATH - path to the bin (only when found)
#
# SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
#
# SPDX-License-Identifier: BSD-2-Clause

find_program(XSettingsd_PATH "xsettingsd")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(XSettingsd
    FOUND_VAR XSettingsd_FOUND
    REQUIRED_VARS XSettingsd_PATH
)
mark_as_advanced(XSettingsd_PATH)

