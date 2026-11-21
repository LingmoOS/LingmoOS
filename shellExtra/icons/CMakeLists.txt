cmake_minimum_required(VERSION 3.16)

project(Lingmo VERSION 1.0 LANGUAGES NONE)

if(DEFINED INSTALL_PREFIX AND NOT INSTALL_PREFIX STREQUAL "")
  set(CMAKE_INSTALL_PREFIX "${INSTALL_PREFIX}" CACHE PATH
      "Installation prefix" FORCE)
elseif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH
      "Installation prefix" FORCE)
endif()

set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "Object code libraries")
include(GNUInstallDirs)

set(LINGMO_THEME_NAME "lingmo-light")
set(LINGMO_ICON_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/icons")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/install-theme.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/install-theme.cmake"
  @ONLY
)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/install-theme.cmake")
