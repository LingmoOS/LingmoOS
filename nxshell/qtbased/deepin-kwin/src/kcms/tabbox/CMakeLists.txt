# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_kwintabbox\")

include_directories(${KWin_SOURCE_DIR}/src/effects ${KWin_SOURCE_DIR}/src/tabbox ${KWin_SOURCE_DIR})

########### next target ###############

set(deepin-kcm_kwintabbox_PART_SRCS
    ${KWin_SOURCE_DIR}/src/tabbox/tabboxconfig.cpp
    layoutpreview.cpp
    main.cpp
    thumbnailitem.cpp
    kwintabboxconfigform.cpp
    kwintabboxdata.cpp
)

ki18n_wrap_ui(deepin-kcm_kwintabbox_PART_SRCS main.ui)
if (QT_MAJOR_VERSION EQUAL "5")
    qt5_add_dbus_interface(deepin-kcm_kwintabbox_PART_SRCS ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Effects.xml kwin_effects_interface)
else()
    qt_add_dbus_interface(deepin-kcm_kwintabbox_PART_SRCS ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Effects.xml kwin_effects_interface)
endif()

kconfig_add_kcfg_files(deepin-kcm_kwintabbox_PART_SRCS deepin-kwintabboxsettings.kcfgc deepin-kwinswitcheffectsettings.kcfgc deepin-kwinpluginssettings.kcfgc)
kcoreaddons_add_plugin(deepin-kcm_kwintabbox SOURCES ${deepin-kcm_kwintabbox_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")

target_link_libraries(deepin-kcm_kwintabbox
    Qt${QT_MAJOR_VERSION}::Quick

    KF5::GlobalAccel
    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuffWidgets
    KF5::Package
    KF5::Service

    XCB::XCB
)

########### install files ###############
install(FILES thumbnails/konqueror.png
              thumbnails/kmail.png
              thumbnails/systemsettings.png
              thumbnails/dolphin.png
              thumbnails/desktop.png
        DESTINATION ${KDE_INSTALL_DATADIR}/deepin-kwin/deepin-kcm_kwintabbox)
install(FILES deepin-kwinswitcher.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
