find_package(PkgConfig REQUIRED)
pkg_get_variable(WlrProtocols_PKGDATADIR wlr-protocols pkgdatadir)
find_package(Qt6 COMPONENTS REQUIRED Core DBus WaylandClient WaylandScannerTools)

add_library(xdg-desktop-portal-dde-wayland SHARED
    portalwaylandcontext.h
    portalwaylandcontext.cpp
    screenshotportal.h
    screenshotportal.cpp
    abstractwaylandportal.h
    protocols/screencopy.h
    protocols/screencopy.cpp
    protocols/common.h
)

qt_generate_wayland_protocol_client_sources(xdg-desktop-portal-dde-wayland FILES
    ${WlrProtocols_PKGDATADIR}/unstable/wlr-screencopy-unstable-v1.xml
)

target_include_directories(xdg-desktop-portal-dde-wayland
PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(xdg-desktop-portal-dde-wayland
PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::GuiPrivate
    Qt6::WaylandClientPrivate
)

install(TARGETS xdg-desktop-portal-dde-wayland DESTINATION ${CMAKE_INSTALL_LIBDIR})