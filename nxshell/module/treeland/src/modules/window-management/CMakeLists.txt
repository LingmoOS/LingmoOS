set(MODULE_NAME window-management)

ws_generate_local(server ${CMAKE_CURRENT_SOURCE_DIR}/protocols/treeland-window-management-v1.xml server-protocol)

pkg_search_module(WLROOTS REQUIRED IMPORTED_TARGET wlroots)

qt_add_library(${MODULE_NAME} STATIC)

target_sources(${MODULE_NAME} PUBLIC
FILE_SET HEADERS
FILES
    windowmanagement.h
    impl/window_management_impl.h
)

target_sources(${MODULE_NAME} PRIVATE
    windowmanagement.cpp
    impl/window_management_impl.cpp
    server-protocol.c
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
        utils
        PkgConfig::WLROOTS
        Waylib::WaylibServer
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Quick
)
