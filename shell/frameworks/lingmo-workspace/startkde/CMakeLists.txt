add_subdirectory(lingmoautostart)
add_subdirectory(kcminit)
add_subdirectory(waitforname)

if (SYSTEMD_FOUND)
    add_subdirectory(systemd)
endif()

qt_add_dbus_interface(
    startlingmo_SRCS
    ${CMAKE_SOURCE_DIR}/ksplash/ksplashqml/org.kde.KSplash.xml
    ksplashinterface
)
ecm_qt_declare_logging_category(startlingmo_SRCS HEADER debug.h IDENTIFIER LINGMO_STARTUP CATEGORY_NAME org.kde.startup)

add_library(startlingmo OBJECT startlingmo.cpp ${startlingmo_SRCS})
target_link_libraries(startlingmo PUBLIC
    Qt::Core
    Qt::DBus
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::Notifications
    KF6::Package
    Canberra::Canberra
    PW::KWorkspace
    lookandfeelmanager
)

add_executable(startlingmo-x11 ${START_LINGMO_COMMON_SRCS} startlingmo-x11.cpp kcheckrunning/kcheckrunning.cpp)
add_executable(startlingmo-wayland ${START_LINGMO_COMMON_SRCS} startlingmo-wayland.cpp)

target_link_libraries(startlingmo-x11 PRIVATE
    startlingmo
    X11::X11 # for kcheckrunning
)

target_link_libraries(startlingmo-wayland PRIVATE
    startlingmo
)

add_subdirectory(lingmo-session)
add_subdirectory(lingmo-shutdown)
add_subdirectory(session-shortcuts)
add_subdirectory(session-restore)

#FIXME: reconsider, looks fishy
if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set_property(SOURCE startlingmo.cpp APPEND PROPERTY COMPILE_DEFINITIONS
        XCURSOR_PATH="${KDE_INSTALL_FULL_DATAROOTDIR}/icons:$XCURSOR_PATH:~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons")
endif()

configure_file(config-startlingmo.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-startlingmo.h)

install(TARGETS startlingmo-x11 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(TARGETS startlingmo-wayland ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS lingmo-sourceenv.sh DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install(PROGRAMS lingmo-dbus-run-session-if-needed DESTINATION ${KDE_INSTALL_LIBEXECDIR})
