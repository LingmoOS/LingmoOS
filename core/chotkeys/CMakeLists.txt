find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(XCB MODULE REQUIRED COMPONENTS XCB KEYSYMS)
find_package(X11)

set(PROJECT_SOURCES
    main.cpp
    application.cpp
    hotkeys.cpp
    QHotkey/qhotkey.cpp
    QHotkey/qhotkey_x11.cpp
)

add_executable(lingmo-chotkeys
    ${PROJECT_SOURCES}
)

target_link_libraries(lingmo-chotkeys
    PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::GuiPrivate
    ${XCB_LIBS}
    ${X11_LIBRARIES}
    XCB::KEYSYMS
)

install(TARGETS lingmo-chotkeys RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
