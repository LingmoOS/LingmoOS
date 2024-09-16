# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_kwin_virtualdesktops\")

########### next target ###############

set(deepin-kcm_kwin_virtualdesktops_PART_SRCS
    ../../virtualdesktopsdbustypes.cpp
    animationsmodel.cpp
    desktopsmodel.cpp
    virtualdesktops.cpp
    virtualdesktopsdata.cpp
)

kconfig_add_kcfg_files(deepin-kcm_kwin_virtualdesktops_PART_SRCS deepin-virtualdesktopssettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(deepin-kcm_kwin_virtualdesktops SOURCES ${deepin-kcm_kwin_virtualdesktops_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(deepin-kcm_kwin_virtualdesktops
    Qt${QT_MAJOR_VERSION}::DBus

    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons
    KF5::XmlGui

    kcmkwincommon
)

########### install files ###############

install(FILES deepin-virtualdesktopssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
kpackage_install_package(package deepin-kcm_kwin_virtualdesktops kcms)
