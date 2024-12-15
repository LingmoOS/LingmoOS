find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt5 COMPONENTS Core Widgets DBus X11Extras REQUIRED)
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
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::X11Extras
    ${XCB_LIBS}
    ${X11_LIBRARIES}
    XCB::KEYSYMS
)

install(TARGETS lingmo-chotkeys RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
