# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_desktoptheme\")

set(kcm_desktoptheme_SRCS
  kcm.cpp
  themesmodel.cpp
  filterproxymodel.cpp
  kcm.h
  themesmodel.h
  filterproxymodel.h
)

kcmutils_generate_module_data(
    kcm_desktoptheme_SRCS
    MODULE_DATA_HEADER desktopthemedata.h
    MODULE_DATA_CLASS_NAME DesktopThemeData
    SETTINGS_HEADERS desktopthemesettings.h
    SETTINGS_CLASSES DesktopThemeSettings
)

kconfig_add_kcfg_files(kcm_desktoptheme_SRCS desktopthemesettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_desktoptheme SOURCES ${kcm_desktoptheme_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(kcm_desktoptheme
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::KIOCore
  KF5::KIOWidgets
  KF5::I18n
  KF5::Plasma
  KF5::Declarative
  KF5::QuickAddons
)

set(lingmo-apply-desktoptheme_SRCS
    lingmo-apply-desktoptheme.cpp
    themesmodel.cpp
)

add_executable(lingmo-apply-desktoptheme ${lingmo-apply-desktoptheme_SRCS})

target_link_libraries(lingmo-apply-desktoptheme
  KF5::CoreAddons
  KF5::KCMUtils
  KF5::I18n
  KF5::Plasma
)

#this desktop file is installed only for retrocompatibility with sycoca
install(FILES lingmo-themes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS lingmo-apply-desktoptheme DESTINATION ${KDE_INSTALL_BINDIR})

kpackage_install_package(package kcm_desktoptheme kcms)
