# Find Avahi. Only avahi-common/defs.h is really needed

# SPDX-FileCopyrightText: 2007 Jakub Stachowski <qbast@go2.pl>
#
# SPDX-License-Identifier: BSD-3-Clause


find_path(AVAHI_INCLUDE_DIR avahi-common/defs.h)
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Avahi DEFAULT_MSG
                                  AVAHI_INCLUDE_DIR)

mark_as_advanced(AVAHI_INCLUDE_DIR)

