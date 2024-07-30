# This module defines the following variables:
#
#  SystemSettings_FOUND - true if found
#  SystemSettings_PATH - path to the bin (only when found)
#
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

include(ProgramFinder)
program_finder(systemsettings
    PATHS ${KDE_INSTALL_FULL_BINDIR} # We symlink by absolute path, so this is the only valid location!
    NO_DEFAULT_PATH
)
