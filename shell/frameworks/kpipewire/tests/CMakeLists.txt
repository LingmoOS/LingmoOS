find_package(Qt6 REQUIRED COMPONENTS WaylandClient)
find_package(Wayland REQUIRED COMPONENTS Client)

add_library(PipeWireScreencastingTest STATIC screencasting.cpp screencastingrequest.cpp)

qt_add_dbus_interface(
   SRCS
   org.freedesktop.portal.ScreenCast.xml
   xdp_dbus_screencast_interface
)

qt_add_dbus_interface(
   SRCS
   org.freedesktop.portal.RemoteDesktop.xml
   xdp_dbus_remotedesktop_interface
)

qt6_generate_wayland_protocol_client_sources(PipeWireScreencastingTest FILES
    ${PLASMA_WAYLAND_PROTOCOLS_DIR}/zkde-screencast-unstable-v1.xml)

target_link_libraries(PipeWireScreencastingTest PUBLIC Wayland::Client Qt::DBus Qt::GuiPrivate KPipeWire Qt::WaylandClient)

add_executable(recordme main.cpp recordme.qrc PlasmaRecordMe.cpp)
target_link_libraries(recordme PipeWireScreencastingTest Qt::Quick)

add_executable(kpipewireheadlesstest HeadlessTest.cpp ${SRCS})
target_link_libraries(kpipewireheadlesstest PipeWireScreencastingTest KPipeWireDmaBuf KPipeWireRecord KF6::CoreAddons)

#########

qt_add_dbus_interface(
   XDP_SRCS
   org.freedesktop.portal.ScreenCast.xml
   xdp_dbus_screencast_interface
)

add_executable(xdp-recordme xdp-main.cpp recordme.qrc ${XDP_SRCS} RecordMe.cpp)
target_link_libraries(xdp-recordme Qt::DBus Qt::Quick KPipeWire Qt::GuiPrivate Wayland::Client Qt::WaylandClient )

option(INSTALL_TESTS "The recordme test needs installing, disabled by default for convenience" OFF)
if(INSTALL_TESTS)
   configure_file(org.kde.recordme.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.recordme.desktop @ONLY)
   configure_file(org.kde.kpipewireheadlesstest.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpipewireheadlesstest.desktop @ONLY)

   install(TARGETS recordme kpipewireheadlesstest DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
   install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.recordme.desktop DESTINATION ${KDE_INSTALL_APPDIR})
   install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kpipewireheadlesstest.desktop DESTINATION ${KDE_INSTALL_APPDIR})
   install(TARGETS xdp-recordme DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

add_executable(webcamtest webcamtest.cpp webcamtest.qml webcamtest.qrc)
target_link_libraries(webcamtest Qt::Quick KPipeWire)
