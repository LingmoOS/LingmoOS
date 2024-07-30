# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

include(ECMAddTests)
include(../KF6KCMUtilsMacros.cmake)

# Ensure we do not actually install the plugins/desktop files
set(KCOREADDONS_INTERNAL_SKIP_PLUGIN_INSTALLATION TRUE)
set(KCMUTILS_INTERNAL_TEST_MODE TRUE)

add_subdirectory(fakekcm)
add_subdirectory(qmlkcm)
add_subdirectory(module_data_codegen)

ecm_add_tests(
    kcmultidialogtest.cpp
    kcmloadtest.cpp
    LINK_LIBRARIES KF6KCMUtils Qt6::Test
)
