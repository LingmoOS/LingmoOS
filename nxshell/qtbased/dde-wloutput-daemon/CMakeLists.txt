cmake_minimum_required(VERSION 3.16)

project(dde-wloutput-daemon)

set(CMAKE_AUTOMOC ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

configure_file(
  misc/org.deepin.dde.KWayland1.service.in
  org.deepin.dde.KWayland1.service
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.KWayland1.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

add_subdirectory(src)
add_subdirectory(test)
