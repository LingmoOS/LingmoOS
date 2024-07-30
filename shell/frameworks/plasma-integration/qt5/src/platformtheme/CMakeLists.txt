find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "Required for updating the Cursor theme on X11"
                        URL "https://www.x.org"
                        TYPE REQUIRED
                        )
set(HAVE_X11 ${X11_FOUND})
if(HAVE_X11)
  find_package(Qt5 ${QT5_MIN_VERSION} CONFIG REQUIRED X11Extras)

  find_package(XCB COMPONENTS XCB)
  set_package_properties(XCB PROPERTIES TYPE REQUIRED)
endif()

configure_file(config-platformtheme.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-platformtheme.h )

# qdbusmenubar uses them
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

set(platformtheme_SRCS
    kdeplatformtheme.cpp kdeplatformtheme.h
    kfontsettingsdata.cpp kfontsettingsdata.h
    khintssettings.cpp khintssettings.h
    kdeplatformfiledialoghelper.cpp kdeplatformfiledialoghelper.h
    kdeplatformfiledialogbase.cpp
    kdeplatformsystemtrayicon.cpp kdeplatformsystemtrayicon.h
    kfiletreeview.cpp
    kdirselectdialog.cpp
    kwaylandintegration.cpp kwaylandintegration.h
    x11integration.cpp x11integration.h
    main.cpp
    qxdgdesktopportalfiledialog.cpp
    qtquickrenderersettings.cpp
)
list(APPEND platformtheme_SRCS qdbusmenubar.cpp) # fork of Qt's qdbusmenubar with some added setters for our convenience

qt5_add_dbus_interface(platformtheme_SRCS org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)
kconfig_add_kcfg_files(platformtheme_SRCS renderersettings.kcfgc)
add_library(KDEPlasmaPlatformTheme5 MODULE)
ecm_add_qtwayland_client_protocol(platformtheme_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/appmenu.xml
    BASENAME appmenu
)

ecm_add_qtwayland_client_protocol(platformtheme_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/server-decoration-palette.xml
    BASENAME server-decoration-palette
)
target_sources(KDEPlasmaPlatformTheme5 PRIVATE ${platformtheme_SRCS})

target_link_libraries(KDEPlasmaPlatformTheme5
    PRIVATE
        Qt5::GuiPrivate
        Qt5::DBus
        Qt5::QuickControls2
        KF5::ConfigCore
        KF5::IconThemes
        KF5::KIOFileWidgets # KFileFilterCombo, KDirSortFilterProxyModel, KRecentDirs
        KF5::KIOWidgets
        KF5::KIOGui
        KF5::XmlGui
        KF5::I18n
        KF5::Notifications
        KF5::WindowSystem
        KF5::GuiAddons
        XCB::XCB
        ${QT5PLATFORMSUPPORT_LIBS}
        Qt5::WaylandClient
        Qt5::GuiPrivate
        Wayland::Client
        KF5::WaylandClient
)
target_link_libraries(KDEPlasmaPlatformTheme5 PRIVATE Qt5::X11Extras)
target_link_libraries(KDEPlasmaPlatformTheme5 PRIVATE KF5::ConfigWidgets)

if(HAVE_X11)
  target_link_libraries(KDEPlasmaPlatformTheme5 PRIVATE ${X11_Xcursor_LIB} ${XCB_XCB_LIBRARY})
endif()

set_target_properties(KDEPlasmaPlatformTheme5 PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/platformthemes/")
install(TARGETS KDEPlasmaPlatformTheme5 DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/platformthemes)
