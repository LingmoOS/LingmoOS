# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkwinscreenedges\")

include_directories(${KWin_SOURCE_DIR}/src/effects)
set(lingmo-kcm_screenedges_SRCS
    monitor.cpp
    screenpreviewwidget.cpp
    kwinscreenedge.cpp
    kwinscreenedgeconfigform.cpp
)

if (QT_MAJOR_VERSION EQUAL "5")
    qt5_add_dbus_interface(lingmo-kcm_screenedges_SRCS ${KWin_SOURCE_DIR}/src/org.lingmo.kwin.Effects.xml kwin_effects_interface)
else()
    qt_add_dbus_interface(lingmo-kcm_screenedges_SRCS ${KWin_SOURCE_DIR}/src/org.lingmo.kwin.Effects.xml kwin_effects_interface)
endif()

set(lingmo-kcm_kwinscreenedges_PART_SRCS main.cpp ${lingmo-kcm_screenedges_SRCS})

kcmutils_generate_module_data(
    lingmo-kcm_kwinscreenedges_PART_SRCS
    MODULE_DATA_HEADER kwinscreenedgedata.h
    MODULE_DATA_CLASS_NAME KWinScreenEdgeData
    SETTINGS_HEADERS kwinscreenedgesettings.h
    SETTINGS_CLASSES KWinScreenEdgeSettings
    NAMESPACE KWin
)

ki18n_wrap_ui(lingmo-kcm_kwinscreenedges_PART_SRCS main.ui)
kconfig_add_kcfg_files(lingmo-kcm_kwinscreenedges_PART_SRCS kwinscreenedgesettings.kcfgc kwinscreenedgescriptsettings.kcfgc kwinscreenedgeeffectsettings.kcfgc)
kcoreaddons_add_plugin(lingmo-kcm_kwinscreenedges SOURCES ${lingmo-kcm_kwinscreenedges_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
set(lingmo-kcm_screenedges_LIBS
    Qt${QT_MAJOR_VERSION}::DBus

    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::KCMUtils
    KF5::I18n
    KF5::Package
    KF5::Plasma
    KF5::Service
)
target_link_libraries(lingmo-kcm_kwinscreenedges ${X11_LIBRARIES} ${lingmo-kcm_screenedges_LIBS})

set(lingmo-kcm_kwintouchscreenedges_PART_SRCS
    touch.cpp
    kwintouchscreenedgeconfigform.cpp
    ${lingmo-kcm_screenedges_SRCS}
)

kcmutils_generate_module_data(
    lingmo-kcm_kwintouchscreenedges_PART_SRCS
    MODULE_DATA_HEADER kwintouchscreendata.h
    MODULE_DATA_CLASS_NAME KWinTouchScreenData
    SETTINGS_HEADERS kwintouchscreensettings.h
    SETTINGS_CLASSES KWinTouchScreenSettings
    NAMESPACE KWin
)

ki18n_wrap_ui(lingmo-kcm_kwintouchscreenedges_PART_SRCS main.ui touch.ui)
kconfig_add_kcfg_files(lingmo-kcm_kwintouchscreenedges_PART_SRCS kwintouchscreensettings.kcfgc kwintouchscreenscriptsettings.kcfgc kwintouchscreenedgeeffectsettings.kcfgc)
kcoreaddons_add_plugin(lingmo-kcm_kwintouchscreen SOURCES ${lingmo-kcm_kwintouchscreenedges_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
target_link_libraries(lingmo-kcm_kwintouchscreen ${X11_LIBRARIES} ${lingmo-kcm_screenedges_LIBS})
