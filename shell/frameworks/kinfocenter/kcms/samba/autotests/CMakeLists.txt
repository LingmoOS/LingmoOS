# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

if(NOT BUILD_TESTING)
    return()
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6Test ${QT_MIN_VERSION} CONFIG REQUIRED)

include(ECMAddTests)

ecm_add_tests(
    ksambasharemodeltest.cpp
    smbmountmodeltest.cpp
    LINK_LIBRARIES
        Qt::Core
        Qt::Test
        kcm_samba_static
)
