# LingmoRunnerSettings Module

set(kcm_krunnersettings_SRCS
    krunnersettings.cpp
)
kcmutils_generate_module_data(
    kcm_krunnersettings_SRCS
    MODULE_DATA_HEADER krunnersettingsdata.h
    MODULE_DATA_CLASS_NAME KRunnerSettingsData
    SETTINGS_HEADERS krunnersettingsbase.h
    SETTINGS_CLASSES KRunnerSettingsBase
)

kconfig_add_kcfg_files(kcm_krunnersettings_SRCS krunnersettingsbase.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_krunnersettings SOURCES ${kcm_krunnersettings_SRCS} INSTALL_NAMESPACE "plasma/kcms/desktop")
kcmutils_generate_desktop_file(kcm_krunnersettings)

target_link_libraries(kcm_krunnersettings
    Qt::DBus
    KF5::Activities
    KF5::ConfigCore
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
)

install(FILES krunnersettingsbase.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
kpackage_install_package(package kcm_krunnersettings kcms)
