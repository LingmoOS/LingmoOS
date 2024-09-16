#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Core Concurrent DBus REQUIRED)
find_package(DtkCore REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(GIO2 REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(XCURSOR REQUIRED IMPORTED_TARGET xcursor)
pkg_check_modules(XFIXES REQUIRED IMPORTED_TARGET xfixes)
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)

# dbus adaptor
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.Session1.xml
    impl/session.h
    Session)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.SessionManager1.xml
    impl/sessionmanager.h
    SessionManager)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.Inhibitor1.xml
    impl/inhibitor.h
    Inhibitor)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.WMSwitcher1.xml
    impl/wmswitcher.h
    WMSwitcher)

# dbus interface
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.PowerManager1.xml PowerManager org_deepin_dde_PowerManager1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.Bluetooth1.xml Bluetooth org_deepin_dde_Bluetooth1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.Daemon1.xml SessionDaemon org_deepin_dde_Daemon1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Manager.xml Login1Manager org_freedesktop_login1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.Session.xml Login1Session org_freedesktop_login1_Session)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.login1.User.xml Login1User org_freedesktop_login1_User)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.DBus.xml DBus org_freedesktop_DBus)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Manager.xml Systemd1Manager org_freedesktop_systemd1_Manager)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.freedesktop.systemd1.Job.xml Systemd1Job org_freedesktop_systemd1_Job)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.Audio1.xml Audio1 org_deepin_dde_Audio1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.Audio1.Sink.xml Sink org_deepin_dde_Audio1_Sink)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES org.deepin.dde.SoundThemePlayer1.xml SoundThemePlayer org_deepin_dde_SoundThemePlayer1)
qt5_add_dbus_interface_fix(INTERFACE_SOURCES com.deepin.wm.xml DeepinWm com_deepin_Wm)

file(GLOB_RECURSE DBUS_TYPES ${PROJECT_SOURCE_DIR}/dbus/types/*)

add_executable(dde-session
    main.cpp
    environmentsmanager.h
    environmentsmanager.cpp
    othersmanager.h
    othersmanager.cpp
    impl/session.h
    impl/session.cpp
    impl/sessionmanager.h
    impl/sessionmanager.cpp
    impl/inhibitor.h
    impl/inhibitor.cpp
    impl/wmswitcher.h
    impl/wmswitcher.cpp
    impl/iowait/iowaitwatcher.h
    impl/iowait/iowaitwatcher.cpp
    impl/iowait/xcursor_remap.h
    impl/iowait/xcursor_remap.c
    ../utils/dconf.h
    ../utils/dconf.cpp
    ../utils/fifo.h
    ../utils/fifo.cpp
    ../utils/utils.h
    ${ADAPTER_SOURCES}
    ${INTERFACE_SOURCES}
    ${DBUS_TYPES}
    )

target_link_libraries(dde-session
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::Systemd
    ${DtkCore_LIBRARIES}
    PkgConfig::GLIB2
    PkgConfig::GIO2
    PkgConfig::QGSettings
    PkgConfig::XCURSOR
    PkgConfig::XFIXES
    PkgConfig::X11
    )

target_include_directories(dde-session PUBLIC
    ../
    ${QT_DBUS_INTERFACE_INCLUDE}
    ${DBUS_INCLUDE}
    )

install(TARGETS dde-session DESTINATION ${CMAKE_INSTALL_BINDIR})
