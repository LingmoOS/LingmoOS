#SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalzez <aleixpol@kde.org>
#SPDX-License-Identifier: BSD-3-Clause

add_definitions(-DTRANSLATION_DOMAIN=\"lingmo-kcm_virtualkeyboard\")

kcmutils_generate_module_data(
    lingmo-kcm_virtualkeyboard_PART_SRCS
    MODULE_DATA_HEADER virtualkeyboarddata.h
    MODULE_DATA_CLASS_NAME VirtualKeyboardData
    SETTINGS_HEADERS lingmo-virtualkeyboardsettings.h
    SETTINGS_CLASSES VirtualKeyboardSettings
)

kconfig_add_kcfg_files(lingmo-kcm_virtualkeyboard_PART_SRCS lingmo-virtualkeyboardsettings.kcfgc GENERATE_MOC)
kcoreaddons_add_plugin(lingmo-kcm_virtualkeyboard SOURCES kcmvirtualkeyboard.cpp ${lingmo-kcm_virtualkeyboard_PART_SRCS} INSTALL_NAMESPACE plasma/kcms/systemsettings)
target_include_directories(lingmo-kcm_virtualkeyboard PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(lingmo-kcm_virtualkeyboard
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

kpackage_install_package(package lingmo-kcm_virtualkeyboard kcms)
