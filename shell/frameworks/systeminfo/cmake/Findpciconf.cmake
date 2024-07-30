# This module defines the following variables:
#
#  pciconf_FOUND - true if found
#  pciconf_PATH - path to the bin (only when found)
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2022 Tobias C. Berner <tcberner@FreeBSD.org>

include(ProgramFinder)
program_finder(pciconf)
