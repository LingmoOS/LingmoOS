add_subdirectory(icons)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/..)

ecm_qt_declare_logging_category(lingmo_discover_SRCS HEADER discover_debug.h IDENTIFIER DISCOVER_LOG CATEGORY_NAME org.kde.lingmo.discover DESCRIPTION "discover" EXPORT DISCOVER)
kconfig_add_kcfg_files(lingmo_discover_SRCS discoversettings.kcfgc GENERATE_MOC)

if (TARGET KF6::UserFeedbackCore)
    kconfig_add_kcfg_files(lingmo_discover_SRCS lingmouserfeedback.kcfgc GENERATE_MOC)
endif()

add_executable(lingmo-discover ${lingmo_discover_SRCS}
    main.cpp
    DiscoverObject.cpp
    DiscoverDeclarativePlugin.cpp

    AbstractAppsModel.cpp
    OdrsAppsModel.cpp
    FeaturedModel.cpp
    PaginateModel.cpp
    UnityLauncher.cpp
    ReadFile.cpp
    PowerManagementInterface.cpp

    DiscoverObject.h
    DiscoverDeclarativePlugin.h

    FeaturedModel.h
    PaginateModel.h
    UnityLauncher.h
    ReadFile.h


    resources.qrc
)
add_executable(Lingmo::Discover ALIAS lingmo-discover)
set_target_properties(lingmo-discover PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/lingmo-discover)

target_link_libraries(lingmo-discover PUBLIC
                                     KF6::Crash
                                     KF6::DBusAddons
                                     KF6::I18n
                                     KF6::ConfigGui
                                     KF6::KIOCore
                                     KF6::WindowSystem
                                     KF6::Notifications
                                     KF6::JobWidgets
                                     KF6::StatusNotifierItem
                                     Qt::Widgets
                                     Qt::Quick
                                     Discover::Common
)

if (TARGET Qt::WebView)
    target_link_libraries(lingmo-discover PUBLIC Qt::WebView)
    target_compile_definitions(lingmo-discover PUBLIC -DWITH_QTWEBVIEW=1)
else()
    target_compile_definitions(lingmo-discover PUBLIC -DWITH_QTWEBVIEW=0)
endif()

if (TARGET KF6::UserFeedbackCore)
    target_link_libraries(lingmo-discover PRIVATE KF6::UserFeedbackCore)
    target_compile_definitions(lingmo-discover PRIVATE WITH_FEEDBACK=1)
endif()

install(TARGETS lingmo-discover ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# if (BUILD_DummyBackend)
#     target_compile_definitions(lingmo-discover PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG=1>)
# endif()

# Standard desktop file accepts local files as input.
set(DesktopNoDisplay "false")
find_program(DPKG dpkg)
find_program(RPM rpm)
set(DesktopMimeType "")
if(DPKG)
     set(DesktopMimeType "${DesktopMimeType}application/vnd.debian.binary-package;")
endif()
if(RPM)
     set(DesktopMimeType "${DesktopMimeType}application/x-rpm;")
endif()
if(Flatpak_FOUND)
  set(DesktopMimeType "${DesktopMimeType}application/vnd.flatpak;application/vnd.flatpak.repo;application/vnd.flatpak.ref;")
endif()
set(DesktopExec "lingmo-discover %F")
configure_file(org.kde.discover.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.desktop)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

# Support appstream:// URI
set(DesktopNoDisplay "true")
set(DesktopMimeType "x-scheme-handler/appstream;")
set(DesktopExec "lingmo-discover %U")
configure_file(org.kde.discover.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.urlhandler.desktop)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.urlhandler.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

# support snap:/ URI
set(DesktopNoDisplay "true")
set(DesktopMimeType "x-scheme-handler/snap;")
set(DesktopExec "lingmo-discover %U")
configure_file(org.kde.discover.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.snap.desktop)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.snap.desktop DESTINATION ${KDE_INSTALL_APPDIR} )

if(EXISTS "/etc/debian_version")
    set(DesktopNoDisplay "true")
    set(DesktopMimeType "x-scheme-handler/apt")
    set(DesktopExec "lingmo-discover %U")
    configure_file(org.kde.discover.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.apt.urlhandler.desktop)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.discover.apt.urlhandler.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
endif()

install(FILES lingmodiscoverui.rc DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/lingmodiscover)
install(FILES org.kde.discover.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
