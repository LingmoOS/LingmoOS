add_subdirectory(plasmaautostart)
add_subdirectory(kcminit)
add_subdirectory(waitforname)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_subdirectory(systemd)
endif()

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

qt_add_dbus_interface(
    startplasma_SRCS
    ${CMAKE_SOURCE_DIR}/ksplash/ksplashqml/org.kde.KSplash.xml
    ksplashinterface
)
ecm_qt_declare_logging_category(startplasma_SRCS HEADER debug.h IDENTIFIER PLASMA_STARTUP CATEGORY_NAME org.kde.startup)

add_library(startplasma OBJECT startplasma.cpp ${startplasma_SRCS})
target_link_libraries(startplasma PUBLIC
    Qt::Core
    Qt::DBus
    KF5::ConfigCore
    KF5::Notifications
    KF5::Package
    ${PHONON_LIBRARIES}
    PW::KWorkspace
    lookandfeelmanager
)

add_executable(startlingmo-x11 ${START_PLASMA_COMMON_SRCS} startlingmo-x11.cpp kcheckrunning/kcheckrunning.cpp)
# add_executable(startlingmo-wayland ${START_PLASMA_COMMON_SRCS} startlingmo-wayland.cpp)

target_link_libraries(startlingmo-x11 PRIVATE
    startplasma
    X11::X11 # for kcheckrunning
)

# target_link_libraries(startlingmo-wayland PRIVATE
#     startplasma
# )

add_subdirectory(lingmo-session)
add_subdirectory(lingmo-poweroff)

#FIXME: reconsider, looks fishy
if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    set_property(SOURCE startplasma.cpp APPEND PROPERTY COMPILE_DEFINITIONS
        XCURSOR_PATH="${KDE_INSTALL_FULL_DATAROOTDIR}/icons:$XCURSOR_PATH:~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons")
endif()

configure_file(config-startplasma.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-startplasma.h)

install(TARGETS startlingmo-x11 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
# install(TARGETS startlingmo-wayland ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS lingmo-sourceenv.sh DESTINATION ${KDE_INSTALL_LIBEXECDIR})
install(PROGRAMS lingmo-dbus-run-session-if-needed DESTINATION ${KDE_INSTALL_LIBEXECDIR})
