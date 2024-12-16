find_package(Qt6 REQUIRED COMPONENTS WaylandClient Widgets)

qt_add_executable(test-show-desktop
    main.cpp
)

qt_generate_wayland_protocol_client_sources(test-show-desktop
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/window-management/protocols/treeland-window-management-v1.xml
)

target_link_libraries(test-show-desktop
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt::WaylandClient
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
)
