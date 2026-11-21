cmake_minimum_required(VERSION 3.18)
project(xdg-desktop-portal-lingmo VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
find_package(LingmoSDK REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
pkg_check_modules(PipeWire REQUIRED IMPORTED_TARGET libpipewire-0.3)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(Gbm REQUIRED IMPORTED_TARGET gbm)
pkg_check_modules(Libdrm REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(WaylandProtocols REQUIRED wayland-protocols>=1.39)
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)

# Client-side marshalling for the capture protocols; generated into the build tree.
set(PROTOCOL_DIR ${CMAKE_BINARY_DIR}/wayland-protocols)
file(MAKE_DIRECTORY ${PROTOCOL_DIR})
function(portal_wayland_protocol target name directory)
    set(xml ${WAYLAND_PROTOCOLS_DATADIR}/${directory}/${name}.xml)
    add_custom_command(OUTPUT ${PROTOCOL_DIR}/${name}-protocol.c
        COMMAND ${WAYLAND_SCANNER} private-code ${xml} ${PROTOCOL_DIR}/${name}-protocol.c
        DEPENDS ${xml} VERBATIM)
    add_custom_command(OUTPUT ${PROTOCOL_DIR}/${name}-client-protocol.h
        COMMAND ${WAYLAND_SCANNER} client-header ${xml} ${PROTOCOL_DIR}/${name}-client-protocol.h
        DEPENDS ${xml} VERBATIM)
    target_sources(${target} PRIVATE
        ${PROTOCOL_DIR}/${name}-protocol.c ${PROTOCOL_DIR}/${name}-client-protocol.h)
endfunction()

# Everything but main(), so the tests run the portals in their own process.
add_library(portal-core STATIC
    src/inhibitportal.cpp
    src/inhibitportal.h
    src/settingsportal.cpp
    src/settingsportal.h
    src/filechooserportal.cpp
    src/filechooserportal.h
    src/filechoosertypes.h
    src/options.cpp
    src/options.h
    src/pipewirecore.cpp
    src/pipewirecore.h
    src/requestobject.cpp
    src/requestobject.h
    src/screencastportal.cpp
    src/screencastportal.h
    src/screencaststream.cpp
    src/screencaststream.h
    src/screencasttypes.h
    src/waylandcapture.cpp
    src/waylandcapture.h
)
portal_wayland_protocol(portal-core ext-image-capture-source-v1 staging/ext-image-capture-source)
portal_wayland_protocol(portal-core ext-image-copy-capture-v1 staging/ext-image-copy-capture)
portal_wayland_protocol(portal-core ext-foreign-toplevel-list-v1 staging/ext-foreign-toplevel-list)
portal_wayland_protocol(portal-core xdg-output-unstable-v1 unstable/xdg-output)
portal_wayland_protocol(portal-core linux-dmabuf-v1 stable/linux-dmabuf)
target_include_directories(portal-core PRIVATE ${PROTOCOL_DIR})
target_include_directories(portal-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(portal-core PUBLIC Lingmo::SDKBase PkgConfig::Systemd
    PRIVATE PkgConfig::PipeWire PkgConfig::WaylandClient PkgConfig::Gbm PkgConfig::Libdrm)

add_executable(xdg-desktop-portal-lingmo src/main.cpp)
target_link_libraries(xdg-desktop-portal-lingmo PRIVATE portal-core)
target_compile_definitions(xdg-desktop-portal-lingmo PRIVATE
    PROJECT_VERSION="${PROJECT_VERSION}"
)

configure_file(data/org.freedesktop.impl.portal.desktop.lingmo.service.in
    org.freedesktop.impl.portal.desktop.lingmo.service @ONLY)
configure_file(data/xdg-desktop-portal-lingmo.service.in
    xdg-desktop-portal-lingmo.service @ONLY)

install(TARGETS xdg-desktop-portal-lingmo RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES data/lingmo.portal DESTINATION ${CMAKE_INSTALL_DATADIR}/xdg-desktop-portal/portals)
install(FILES data/lingmo-portals.conf DESTINATION ${CMAKE_INSTALL_DATADIR}/xdg-desktop-portal)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.impl.portal.desktop.lingmo.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
set(SYSTEMD_USER_UNIT_DIR "lib/systemd/user" CACHE STRING
    "Installation directory for systemd user units")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xdg-desktop-portal-lingmo.service
    DESTINATION ${SYSTEMD_USER_UNIT_DIR})

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
