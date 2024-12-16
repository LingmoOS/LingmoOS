find_package(Qt6 REQUIRED COMPONENTS WaylandClient Widgets DBus)
find_package(PkgConfig)
pkg_search_module(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
pkg_get_variable(WAYLAND_PROTOCOLS wayland-protocols pkgdatadir)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

qt_add_executable(treeland-shortcut
    shortcut.h
    shortcut.cpp
)

qt_generate_wayland_protocol_client_sources(treeland-shortcut
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../modules/shortcut/protocols/treeland-shortcut-manager-v1.xml
)

target_link_libraries(treeland-shortcut
    PRIVATE
        Waylib::WaylibServer
        Qt::Widgets
        Qt::DBus
        Qt::WaylandClient
        Qt::WaylandClientPrivate
        PkgConfig::WAYLAND_CLIENT
        PkgConfig::Systemd
)

install(TARGETS treeland-shortcut RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")
install(DIRECTORY "shortcuts" DESTINATION "${TREELAND_DATA_DIR}")
