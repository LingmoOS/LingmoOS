find_package(Qt6 REQUIRED COMPONENTS WaylandClient Widgets)

qt_add_executable(test-window-bg
    main.cpp
)

qt_generate_wayland_protocol_client_sources(test-window-bg
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/personalization/protocols/treeland-personalization-manager-v1.xml
)

target_link_libraries(test-window-bg
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt::WaylandClient
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
)
