cmake_minimum_required(VERSION 3.16)

project(ocean-wloutput-daemon)

set(CMAKE_AUTOMOC ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

configure_file(
  misc/org.lingmo.ocean.KWayland1.service.in
  org.lingmo.ocean.KWayland1.service
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.ocean.KWayland1.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

add_subdirectory(src)
add_subdirectory(test)
