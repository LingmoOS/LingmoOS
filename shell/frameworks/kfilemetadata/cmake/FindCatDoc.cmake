# - Find CatDoc
#
# This module defines
#  CATDOC_FOUND - whether the catdoc executable was found

# SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

find_program(CATDOC_EXE NAMES catdoc)
if (CATDOC_EXE)
    set (CATDOC_FOUND TRUE)
endif()

