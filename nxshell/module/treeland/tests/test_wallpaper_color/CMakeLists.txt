find_package(Qt6 REQUIRED COMPONENTS Core WaylandClient)

qt_add_executable(test-wallpaper-color
    main.cpp
)

qt_generate_wayland_protocol_client_sources(test-wallpaper-color
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../../src/modules/wallpaper-color/protocols/treeland-wallpaper-color-v1.xml
)

target_link_libraries(test-wallpaper-color
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Gui
        Qt::WaylandClient
        Qt::GuiPrivate
        Qt::WaylandClientPrivate
)
