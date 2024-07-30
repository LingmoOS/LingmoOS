# FindSCPDbus
# ------------
#
# Try to find system-config-printer-dbus-service. This is generally installed with
# the system-config-printer package but can also be installed independently.
#
# This will define the following variables:
#
# ``SCPDbus_FOUND``
#     True if (the requested version of) system-config-printer-dbus-service is available
# ``SCPDbus_BACKEND``
#     Path of scp dbus service

# SPDX-FileCopyrightText: 2024 Mike Noe <noeerover@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

find_file(SCPDbus_BACKEND scp-dbus-service
    PATHS
        /usr/bin/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SCPDbus
    FOUND_VAR
        SCPDbus_FOUND
    REQUIRED_VARS
        SCPDbus_BACKEND
)
