# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_colors\")

set(kcm_colors_SRCS
    ../kcms-common.cpp
    colors.cpp
    colorsapplicator.cpp
    colorsmodel.cpp
    filterproxymodel.cpp
)

qt_generate_dbus_interface(
    accentColorService/accentColorService.h
    org.lingmo.lingmoshell.accentColor.xml
    OPTIONS -M
)

set(lingmo-accentcolor-service_SRCS
    accentColorService/accentColorService.cpp
    colorsapplicator.cpp
    ../kcms-common.cpp
    ../krdb/krdb.cpp
)


set(lingmo-accentcolor-service_SRCS
    ${lingmo-accentcolor-service_SRCS}
    ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.lingmoshell.accentColor.xml
)

qt_add_dbus_adaptor(lingmo-accentcolor-service_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.lingmoshell.accentColor.xml
                     accentColorService/accentColorService.h AccentColorService
                     accentcolor_service_adaptor AccentColorServiceAdaptor)

kcmutils_generate_module_data(
    kcm_colors_SRCS
    MODULE_DATA_HEADER colorsdata.h
    MODULE_DATA_CLASS_NAME ColorsData
    SETTINGS_HEADERS colorssettings.h
    SETTINGS_CLASSES ColorsSettings
)

# needed for krdb
kconfig_add_kcfg_files(kcm_colors_SRCS colorssettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(kcm_colors SOURCES ${kcm_colors_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file (kcm_colors)
target_link_libraries(kcm_colors
    Qt::DBus
    KF5::KCMUtils
    KF5::CoreAddons
    KF5::Declarative
    KF5::GuiAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::NewStuffCore
    KF5::QuickAddons
    KF5::WindowSystem
    krdb
)

if(HAVE_X11)
    target_link_libraries(kcm_colors X11::X11)
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(kcm_colors Qt::X11Extras)
    else()
        target_link_libraries(kcm_colors Qt::GuiPrivate)
    endif()
endif()

set(lingmo-apply-colorscheme_SRCS
    lingmo-apply-colorscheme.cpp
    colorsapplicator.cpp
    colorsmodel.cpp
    ../kcms-common.cpp
    ../krdb/krdb.cpp
)

kconfig_add_kcfg_files(lingmo-apply-colorscheme_SRCS colorssettings.kcfgc GENERATE_MOC)
kconfig_add_kcfg_files(lingmo-accentcolor-service_SRCS colorssettings.kcfgc GENERATE_MOC)

add_executable(lingmo-apply-colorscheme ${lingmo-apply-colorscheme_SRCS})
kcoreaddons_add_plugin(plasma_accentcolor_service SOURCES ${lingmo-accentcolor-service_SRCS} INSTALL_NAMESPACE "kf${QT_MAJOR_VERSION}/kded")

target_link_libraries(lingmo-apply-colorscheme
    Qt::Core
    Qt::DBus
    Qt::Gui
    KF5::GuiAddons
    KF5::KCMUtils
    KF5::I18n
    KF5::WindowSystem
    PW::KWorkspace
    X11::X11
)

target_link_libraries(plasma_accentcolor_service
    Qt::Core
    Qt::DBus
    KF5::GuiAddons
    KF5::KCMUtils
    KF5::I18n
    KF5::WindowSystem
    KF5::ConfigCore
    KF5::DBusAddons
    PW::KWorkspace
    X11::X11
)


if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(lingmo-apply-colorscheme Qt::X11Extras)
else()
    target_link_libraries(lingmo-apply-colorscheme Qt::GuiPrivate)
endif()

if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(plasma_accentcolor_service Qt::X11Extras)
else()
    target_link_libraries(plasma_accentcolor_service Qt::GuiPrivate)
endif()

install(FILES colorssettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(TARGETS lingmo-apply-colorscheme DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES colorschemes.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

kpackage_install_package(package kcm_colors kcms)

add_subdirectory(editor)
