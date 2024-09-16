include_directories(
    ${LIBXAU_INCLUDE_DIRS}
    "${LIBXCB_INCLUDE_DIR}"
)

configure_file(config.h.in config.h IMMEDIATE @ONLY)
set(DAEMON_SOURCES
    DaemonApp.cpp
    Display.cpp
    DisplayManager.cpp
    DisplayServer.cpp
    Greeter.cpp
    PowerManager.cpp
    Seat.cpp
    SeatManager.cpp
    SocketServer.cpp
    XorgDisplayServer.cpp
    XorgUserDisplayServer.cpp
    XorgUserDisplayServer.h
    WaylandDisplayServer.cpp
    WaylandDisplayServer.h
    SingleWaylandDisplayServer.h
    SingleWaylandDisplayServer.cpp
)

qt_add_dbus_adaptor(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.xml"          "DisplayManager.h" DDM::DisplayManager)
qt_add_dbus_adaptor(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Seat.xml"     "DisplayManager.h" DDM::DisplayManagerSeat)
qt_add_dbus_adaptor(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.DisplayManager.Session.xml"  "DisplayManager.h" DDM::DisplayManagerSession)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Manager.xml" PROPERTIES
   INCLUDE "LogindDBusTypes.h"
)
set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Seat.xml" PROPERTIES
   INCLUDE "LogindDBusTypes.h"
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Session.xml" PROPERTIES
   INCLUDE "LogindDBusTypes.h"
)

set_source_files_properties("${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.User.xml" PROPERTIES
   INCLUDE "LogindDBusTypes.h"
)

qt_add_dbus_interface(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Manager.xml"  "Login1Manager")
qt_add_dbus_interface(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Seat.xml"  "Login1Seat")
qt_add_dbus_interface(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.Session.xml"  "Login1Session")
qt_add_dbus_interface(DAEMON_SOURCES "${CMAKE_SOURCE_DIR}/data/interfaces/org.freedesktop.login1.User.xml"  "Login1User")

add_executable(ddm ${DAEMON_SOURCES})
target_link_libraries(ddm
    PRIVATE
        auth
    PUBLIC
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Network
        Qt${QT_MAJOR_VERSION}::Qml
        ${LIBXCB_LIBRARIES}
)

if(JOURNALD_FOUND)
    target_link_libraries(ddm
        PUBLIC
            ${JOURNALD_LIBRARIES}
    )
endif()

install(TARGETS ddm DESTINATION "${CMAKE_INSTALL_BINDIR}")
