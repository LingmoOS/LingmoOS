# This module defines the following variables:
#
#  lsblk_FOUND - true if found
#  lsblk_PATH - path to the bin (only when found)
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Harald Sitter <sitter@kde.org>

include(ProgramFinder)
program_finder(lsblk)
