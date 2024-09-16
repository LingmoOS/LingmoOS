#SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalzez <aleixpol@kde.org>
#SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_virtualkeyboard\")

kcmutils_generate_module_data(
    deepin-kcm_virtualkeyboard_PART_SRCS
    MODULE_DATA_HEADER virtualkeyboarddata.h
    MODULE_DATA_CLASS_NAME VirtualKeyboardData
    SETTINGS_HEADERS deepin-virtualkeyboardsettings.h
    SETTINGS_CLASSES VirtualKeyboardSettings
)

kconfig_add_kcfg_files(deepin-kcm_virtualkeyboard_PART_SRCS deepin-virtualkeyboardsettings.kcfgc GENERATE_MOC)
kcoreaddons_add_plugin(deepin-kcm_virtualkeyboard SOURCES kcmvirtualkeyboard.cpp ${deepin-kcm_virtualkeyboard_PART_SRCS} INSTALL_NAMESPACE plasma/kcms/systemsettings)
target_include_directories(deepin-kcm_virtualkeyboard PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(deepin-kcm_virtualkeyboard
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

kpackage_install_package(package deepin-kcm_virtualkeyboard kcms)
