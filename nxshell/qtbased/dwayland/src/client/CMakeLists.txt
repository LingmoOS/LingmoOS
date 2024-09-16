remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

# needed to access QPA
include_directories(SYSTEM ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

set(CLIENT_LIB_SRCS
    appmenu.cpp
    buffer.cpp
    blur.cpp
    clientmanagement.cpp
    compositor.cpp
    connection_thread.cpp
    contrast.cpp
    slide.cpp
    event_queue.cpp
    datacontroldevice.cpp
    datacontroldevicemanager.cpp
    datacontroloffer.cpp
    datacontrolsource.cpp
    datadevice.cpp
    datadevicemanager.cpp
    dataoffer.cpp
    datasource.cpp
    ddeseat.cpp
    ddekeyboard.cpp
    ddeshell.cpp
    dpms.cpp
    fakeinput.cpp
    fullscreen_shell.cpp
    idle.cpp
    idleinhibit.cpp
    keyboard.cpp
    keystate.cpp
    remote_access.cpp
    outputconfiguration.cpp
    outputconfiguration_v2.cpp
    outputmanagement.cpp
    outputmanagement_v2.cpp
    outputdevice.cpp
    outputdevicemode_v2.cpp
    outputdevice_v2.cpp
    output.cpp
    pointer.cpp
    pointerconstraints.cpp
    pointergestures.cpp
    plasmashell.cpp
    plasmavirtualdesktop.cpp
    plasmawindowmanagement.cpp
    plasmawindowmodel.cpp
    primaryoutput_v1.cpp
    region.cpp
    registry.cpp
    relativepointer.cpp
    seat.cpp
    server_decoration.cpp
    server_decoration_palette.cpp
    shadow.cpp
    shell.cpp
    shm_pool.cpp
    strut.cpp
    subcompositor.cpp
    subsurface.cpp
    surface.cpp
    touch.cpp
    textinput.cpp
    textinput_v0.cpp
    textinput_v2.cpp
    xdgdecoration.cpp
    xdgshell.cpp
    xdgforeign_v2.cpp
    xdgforeign.cpp
    xdgshell_v5.cpp
    xdgshell_v6.cpp
    xdgshell_stable.cpp
    xdgoutput.cpp
    globalproperty.cpp
    ../compat/wayland-xdg-shell-v5-protocol.c
)

ecm_qt_declare_logging_category(CLIENT_LIB_SRCS
    HEADER logging.h
    IDENTIFIER KWAYLAND_CLIENT
    CATEGORY_NAME dwayland.client
    OLD_CATEGORY_NAMES kwayland-client
    DEFAULT_SEVERITY Critical
    DESCRIPTION "DWayland Client Library"
    EXPORT DWAYLAND
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/fullscreen-shell.xml
    BASENAME fullscreen-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/output-management.xml
    BASENAME output-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/outputdevice.xml
    BASENAME org_kde_kwin_outputdevice
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/kde-output-device-v2.xml
    BASENAME kde-output-device-v2
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/kde-output-management-v2.xml
    BASENAME kde-output-management-v2
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/kde-primary-output-v1.xml
    BASENAME kde-primary-output-v1
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/plasma-shell.xml
    BASENAME plasma-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/plasma-virtual-desktop.xml
    BASENAME plasma-virtual-desktop
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/plasma-window-management.xml
    BASENAME plasma-window-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/idle.xml
    BASENAME idle
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/fake-input.xml
    BASENAME fake-input
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/shadow.xml
    BASENAME shadow
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/blur.xml
    BASENAME blur
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/contrast.xml
    BASENAME contrast
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/slide.xml
    BASENAME slide
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/dpms.xml
    BASENAME dpms
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/server-decoration.xml
    BASENAME server-decoration
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/text-input.xml
    BASENAME text-input-v0
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/text-input-unstable-v2.xml
    BASENAME text-input-v2
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-shell/xdg-shell-unstable-v6.xml
    BASENAME xdg-shell-v6
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/relative-pointer/relative-pointer-unstable-v1.xml
    BASENAME relativepointer-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/pointer-gestures/pointer-gestures-unstable-v1.xml
    BASENAME pointer-gestures-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
    BASENAME pointer-constraints-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-foreign/xdg-foreign-unstable-v2.xml
    BASENAME xdg-foreign-unstable-v2
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/idle-inhibit/idle-inhibit-unstable-v1.xml
    BASENAME idle-inhibit-unstable-v1
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/appmenu.xml
    BASENAME appmenu
)
ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/server-decoration-palette.xml
    BASENAME server-decoration-palette
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-output/xdg-output-unstable-v1.xml
    BASENAME xdg-output-unstable-v1
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
    BASENAME xdg-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
    BASENAME xdg-decoration-unstable-v1
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/keystate.xml
    BASENAME keystate
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/client-management.xml
    BASENAME client-management
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/dde-seat.xml
    BASENAME dde-seat
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/dde-shell.xml
    BASENAME dde-shell
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/dde-globalproperty.xml
    BASENAME dde-globalproperty
)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/strut.xml
    BASENAME strut
)

ecm_add_wayland_client_protocol (CLIENT_LIB_SRCS
    PROTOCOL ${PROJECT_SOURCE_DIR}/src/protocols/wlr-data-control-unstable-v1.xml
    BASENAME wlr-data-control-unstable-v1
)

set(CLIENT_GENERATED_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-fullscreen-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-output-management-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-org_kde_kwin_outputdevice-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-plasma-window-management-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-idle-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-fake-input-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-shadow-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-blur-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-strut-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-contrast-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-slide-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-dpms-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-server-decoration-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-server-decoration-palette-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-text-input-v0-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-text-input-v2-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-shell-v6-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-relativepointer-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-pointer-gestures-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-pointer-constraints-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-foreign-unstable-v2-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-idle-inhibit-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-output-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-xdg-decoration-unstable-v1-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-client-management-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-dde-seat-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-dde-shell-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-dde-globalproperty-client-protocol.h
    ${CMAKE_CURRENT_BINARY_DIR}/wayland-wlr-data-control-unstable-v1-protocol.h
)

set_source_files_properties(${CLIENT_GENERATED_FILES} PROPERTIES SKIP_AUTOMOC ON)

ecm_add_wayland_client_protocol(CLIENT_LIB_SRCS
    PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/remote-access.xml
    BASENAME remote-access
)

add_library(DWaylandClient ${CLIENT_LIB_SRCS})
add_library(Deepin::WaylandClient ALIAS DWaylandClient)
ecm_generate_export_header(DWaylandClient
    BASE_NAME
        KWaylandClient
    EXPORT_FILE_NAME
        DWayland/Client/kwaylandclient_export.h
    GROUP_BASE_NAME KF
    VERSION ${PROJECT_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.49 5.50 5.52 5.53 5.73 5.82
)
# TODO: add support for EXCLUDE_DEPRECATED_BEFORE_AND_AT to all KWayland libs
# needs fixing of undeprecated API being still implemented using own deprecated API

target_include_directories(DWaylandClient INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>/DWayland/Client")

target_link_libraries(DWaylandClient
    PUBLIC Qt5::Gui
    PRIVATE Wayland::Client
        Qt5::Concurrent
)

set_target_properties(DWaylandClient PROPERTIES VERSION   ${DWAYLAND_VERSION}
                                                 SOVERSION ${DWAYLAND_SOVERSION}
                                                 EXPORT_NAME WaylandClient
)

install(TARGETS DWaylandClient EXPORT DWaylandTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

set(CLIENT_LIB_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/DWayland/Client/kwaylandclient_export.h
  appmenu.h
  blur.h
  buffer.h
  clientmanagement.h
  compositor.h
  connection_thread.h
  contrast.h
  event_queue.h
  datacontroldevice.h
  datacontroldevicemanager.h
  datacontroloffer.h
  datacontrolsource.h
  datadevice.h
  datadevicemanager.h
  dataoffer.h
  datasource.h
  ddeseat.h
  ddekeyboard.h
  ddeshell.h
  dpms.h
  fakeinput.h
  fullscreen_shell.h
  idle.h
  idleinhibit.h
  keyboard.h
  keystate.h
  remote_access.h
  outputconfiguration.h
  outputconfiguration_v2.h
  outputmanagement.h
  outputmanagement_v2.h
  outputdevice.h
  outputdevice_v2.h
  outputdevicemode_v2.h
  output.h
  pointer.h
  pointerconstraints.h
  plasmashell.h
  plasmavirtualdesktop.h
  plasmawindowmanagement.h
  plasmawindowmodel.h
  pointergestures.h
  primaryoutput_v1.h
  region.h
  registry.h
  relativepointer.h
  seat.h
  server_decoration.h
  server_decoration_palette.h
  shadow.h
  shell.h
  shm_pool.h
  slide.h
  strut.h
  subcompositor.h
  subsurface.h
  surface.h
  touch.h
  textinput.h
  xdgdecoration.h
  xdgshell.h
  xdgforeign.h
  xdgoutput.h
  globalproperty.h
)

install(FILES
  ${CLIENT_LIB_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/DWayland/Client COMPONENT Devel
)

# make available to ecm_add_qch in parent folder
set(DWaylandClient_APIDOX_SRCS ${CLIENT_LIB_HEADERS} PARENT_SCOPE)
set(DWaylandClient_APIDOX_BUILD_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME DWaylandClient LIB_NAME DWaylandClient DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR})
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
