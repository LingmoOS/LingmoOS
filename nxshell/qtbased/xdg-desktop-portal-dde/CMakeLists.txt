cmake_minimum_required(VERSION 3.16)

project(xdg-desktop-portal-dde
    VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

add_subdirectory(src)

configure_file(
    misc/xdg-desktop-portal-dde.service.in
    xdg-desktop-portal-dde.service
    @ONLY)

configure_file(
    misc/org.freedesktop.impl.portal.desktop.dde.service.in
    org.freedesktop.impl.portal.desktop.dde.service
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xdg-desktop-portal-dde.service DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/systemd/user/")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.impl.portal.desktop.dde.service DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/services")
install(FILES ${CMAKE_CURRENT_LIST_DIR}/misc/dde.portal DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/xdg-desktop-portal/portals/")
install(FILES ${CMAKE_CURRENT_LIST_DIR}/misc/dde-portals.conf DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/xdg-desktop-portal/")
