find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS X11Extras)
pkg_check_modules(XCB IMPORTED_TARGET xcb)

set(BIN_NAME frameless-window)

add_executable(${BIN_NAME}
    main.cpp
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::X11Extras
    PkgConfig::XCB
)
