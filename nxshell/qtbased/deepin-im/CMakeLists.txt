cmake_minimum_required(VERSION 3.16)

project(deepin-im VERSION 1.0 LANGUAGES C CXX)

include(GNUInstallDirs)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
option(ENABLE_GTK3_IM_MODULE "Enable GTK3 IM Module" ON)
option(ENABLE_GTK4_IM_MODULE "Enable GTK4 IM Module" ON)

find_package(ECM 1.4.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Widgets Xml DBus)
find_package(Qt6Gui ${REQUIRED_QT6_VERSION} REQUIRED Private)
find_package(Fcitx5Qt6DBusAddons REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(WaylandScanner REQUIRED)
find_package(Dtk6Core QUIET)
find_package(Dtk6Gui QUIET)

if (Dtk6Core_FOUND)
  add_definitions(-DDtk6Core_FOUND)
endif()

if (Dtk6Gui_FOUND)
  add_definitions(-DDtk6Gui_FOUND)
endif()

add_definitions(-DWLR_USE_UNSTABLE)

pkg_check_modules(xcb REQUIRED IMPORTED_TARGET xcb)
pkg_check_modules(xcb-xinput REQUIRED IMPORTED_TARGET xcb-xinput)
pkg_check_modules(xcb-aux REQUIRED IMPORTED_TARGET xcb-aux)
pkg_check_modules(xkbcommon REQUIRED IMPORTED_TARGET xkbcommon)
pkg_check_modules(xkeyboard-config REQUIRED IMPORTED_TARGET xkeyboard-config)
pkg_check_modules(wayland-client REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(wayland-server REQUIRED IMPORTED_TARGET wayland-server)
pkg_check_modules(libudev REQUIRED IMPORTED_TARGET libudev)
pkg_check_modules(libinput REQUIRED IMPORTED_TARGET libinput)
pkg_check_modules(gio REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(wlroots REQUIRED IMPORTED_TARGET wlroots)

pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)

if (ENABLE_GTK3_IM_MODULE)
    pkg_check_modules(Gtk3 REQUIRED IMPORTED_TARGET "gtk+-3.0")
    pkg_check_modules(Gdk3 REQUIRED IMPORTED_TARGET "gdk-3.0")
    pkg_get_variable(GTK3_BINARY_VERSION "gtk+-3.0" "gtk_binary_version")
    pkg_get_variable(GTK3_TARGETS "gtk+-3.0" "targets")
endif()

if (ENABLE_GTK4_IM_MODULE)
    pkg_check_modules(Gtk4 REQUIRED IMPORTED_TARGET "gtk4>=4.2")
    pkg_get_variable(GTK4_BINARY_VERSION "gtk4" "gtk_binary_version")
    pkg_get_variable(GTK4_TARGETS "gtk4" "targets")
endif()

add_library(wayland_protocols OBJECT)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL ${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml
  BASENAME xdg-shell
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL misc/wayland-protocols/input-method-unstable-v1.xml
  BASENAME input-method-unstable-v1
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL misc/wayland-protocols/input-method-unstable-v2.xml
  BASENAME input-method-unstable-v2
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL misc/wayland-protocols/virtual-keyboard-unstable-v1.xml
  BASENAME virtual-keyboard-unstable-v1
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL ${WAYLAND_PROTOCOLS_DIR}/unstable/text-input/text-input-unstable-v3.xml
  BASENAME text-input-unstable-v3
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL misc/wayland-protocols/wlr-foreign-toplevel-management-unstable-v1.xml
  BASENAME wlr-foreign-toplevel-management-unstable-v1
)

ecm_add_wayland_server_protocol(wayland_protocols
  PROTOCOL misc/wayland-protocols/treeland-foreign-toplevel-manager-v1.xml
  BASENAME treeland-foreign-toplevel-manager-server-protocol-v1
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(misc)
add_subdirectory(demo)
