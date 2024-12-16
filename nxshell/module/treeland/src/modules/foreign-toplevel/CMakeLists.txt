set(MODULE_NAME foreign-toplevel)

ws_generate_local(server ${CMAKE_CURRENT_SOURCE_DIR}/protocols/treeland-foreign-toplevel-manager-v1.xml server-protocol)
ws_generate(server wayland-protocols stable/xdg-shell/xdg-shell.xml xdg-shell-protocol)

pkg_search_module(WLROOTS REQUIRED IMPORTED_TARGET wlroots)

qt_add_library(${MODULE_NAME} STATIC)

target_sources(${MODULE_NAME} PUBLIC
FILE_SET HEADERS
FILES
    foreigntoplevelmanagerv1.h
    impl/foreign_toplevel_manager_impl.h
)

target_sources(${MODULE_NAME} PRIVATE
    foreigntoplevelmanagerv1.cpp
    impl/foreign_toplevel_manager_impl.cpp
    server-protocol.c
    xdg-shell-protocol.c
)

target_include_directories(${MODULE_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_definitions(${MODULE_NAME}
    PRIVATE
    WLR_USE_UNSTABLE
)

target_link_libraries(${MODULE_NAME}
    PUBLIC
        PkgConfig::WLROOTS
        Waylib::WaylibServer
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Quick
)
