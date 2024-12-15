add_executable(dimwl
    main.cpp
    Listener.h
    Server.h
    Server.cpp
    Output.h
    Output.cpp
    View.h
    View.cpp
    Keyboard.h
    Keyboard.cpp
    InputMethodV2.h
    InputMethodV2.cpp
    TextInputV3.h
    TextInputV3.cpp
    XcbHelper.h
    XcbHelper.cpp
    Xcb.h
    Xcb.cpp
    X11ActiveWindowMonitor.h
    X11ActiveWindowMonitor.cpp
    X11KeyboardGrabber.h
    X11KeyboardGrabber.cpp
)


include(${PROJECT_SOURCE_DIR}/cmake/WaylandScannerHelpers.cmake)

ws_generate(
    server
    wayland-protocols
    stable/xdg-shell/xdg-shell.xml
    xdg-shell-protocol
)

target_link_libraries(dimwl PUBLIC
    PkgConfig::wlroots
    PkgConfig::wayland-server
    PkgConfig::xkbcommon
    PkgConfig::xcb
    PkgConfig::xcb-aux
    PkgConfig::xcb-xinput
)

install(TARGETS dimwl
    BUNDLE BUNDLE DESTINATION .
)
