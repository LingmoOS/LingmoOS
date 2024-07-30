# FindCupsSmb
# ------------
#
# Try to find smb backend for cups. This is generally installed as smbspool but
# needs symlinking into place for CUPS (by the distribution).
#
# This will define the following variables:
#
# ``CupsSmb_FOUND``
#     True if (the requested version of) CupsSmb is available
# ``CupsSmb_BACKEND``
#     Path of smb backend file

# SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

find_file(CupsSmb_BACKEND smb
    PATHS
        /lib/cups/backend/
        /usr/lib/cups/backend/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CupsSmb
    FOUND_VAR
        CupsSmb_FOUND
    REQUIRED_VARS
        CupsSmb_BACKEND
)
