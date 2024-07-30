# SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

include(../../KF6KCMUtilsMacros.cmake)

set(_KCMODULE_DATA_TEMPLATE_CPP "../../src/kcmutilsgeneratemoduledata.cpp.in")
set(_KCMODULE_DATA_TEMPLATE_H   "../../src/kcmutilsgeneratemoduledata.h.in")
set(module_data_test_SRCS module_data_test.cpp)
kcmutils_generate_module_data(
        module_data_test_SRCS
        MODULE_DATA_HEADER cursorthemedata.h
        MODULE_DATA_CLASS_NAME CursorThemeData
        SETTINGS_HEADERS cursorthemesettings.h
        SETTINGS_CLASSES CursorThemeSettings
        NAMESPACE Test
)
kconfig_add_kcfg_files(module_data_test_SRCS cursorthemesettings.kcfgc GENERATE_MOC)

ecm_add_test(${module_data_test_SRCS}
        TEST_NAME module_data_test
        LINK_LIBRARIES KF6KCMUtils Qt6::Test
)
