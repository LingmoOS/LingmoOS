set(WAYLANDSERVER_SOURCES
  WaylandServer_public.h
  WaylandServer_public.cpp
  WaylandServer.h
  WaylandServer.cpp
)

add_library(waylandserver MODULE
  ${WAYLANDSERVER_SOURCES}
)

include(${PROJECT_SOURCE_DIR}/cmake/WaylandScannerHelpers.cmake)

ws_generate(
    server
    wayland-protocols
    stable/xdg-shell/xdg-shell.xml
    xdg-shell-protocol
)

target_link_libraries(waylandserver PRIVATE
  dimcore
  PkgConfig::wlroots
  PkgConfig::wayland-server
  wlc
  Qt6::Core
  wl_addons_base
)

install(TARGETS waylandserver
  LIBRARY DESTINATION ${DIM_ADDON_DIR}
)

install(FILES waylandserver.conf
  DESTINATION ${DIM_ADDON_INFO_DIR}
)

