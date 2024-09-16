set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_compile_options(-DQT_NO_KEYWORDS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Concurrent Gui)
find_package(DtkCore REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5Screen REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

pkg_check_modules(X11 REQUIRED IMPORTED_TARGET xcursor xfixes x11)
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb-render xcb xcb-randr xcb-cursor)

macro(qt5_add_dbus_interface_fix srcs xml class file)
  execute_process(COMMAND qdbusxml2cpp-fix -c ${class} -p ${PROJECT_BINARY_DIR}/dbusinterface/${file} ${PROJECT_SOURCE_DIR}/dbus/interface/${xml})
  list(APPEND ${srcs}
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.h
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.cpp
  )
endmacro(qt5_add_dbus_interface_fix)

# dbus interface


# dbus adaptor
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.Display1.xml
    display.h
    Display1)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
    ../../dbus/adaptor/org.deepin.dde.Display1.Monitor.xml
    monitor.h
    Monitor)
add_definitions(-DUSE_QT)

file(GLOB_RECURSE DBUS_TYPES
    ../dbus/brightnessmap.h
    ../dbus/brightnessmap.cpp
    ../dbus/resolution.h
    ../dbus/resolution.cpp
    ../dbus/resolutionlist.h
    ../dbus/resolutionlist.cpp
    ../dbus/screenrect.h
    ../dbus/screenrect.cpp
    ../dbus/touchscreeninfolist.h
    ../dbus/touchscreeninfolist.cpp
    ../dbus/touchscreeninfolist_v2.h
    ../dbus/touchscreeninfolist_v2.cpp
    ../dbus/touchscreenmap.h
    ../dbus/touchscreenmap.cpp
    ../dbus/reflectlist.h
    ../dbus/reflectlist.cpp
    ../dbus/rotationlist.h
    ../dbus/rotationlist.cpp
)

set(SRCS
    main.cpp
    config.cpp
    display.cpp
    monitor.cpp
    displaymanager.cpp
    ../common/control.cpp
    ../common/control.h
    ../common/globals.cpp
    ../common/globals.h
    ../common/utils.cpp
    ../common/utils.h
    ${DBUS_TYPES}
)

add_executable(dde-display
    ${SRCS}
    ${ADAPTER_SOURCES}
)

target_include_directories(dde-display PUBLIC
    PkgConfig::X11
    PkgConfig::XCB
    ${X11_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${KF5Screen_INCLUDE_DIRS}
    ${QT_DBUS_INTERFACE_INCLUDE}
    ${DBUSEXTENDED_INCLUDE}
)

target_link_libraries(dde-display PRIVATE
    Qt5::Core
    Qt5::DBus
    Qt5::Concurrent
    Qt5::Gui
    PkgConfig::X11
    PkgConfig::XCB
    KF5::Screen
    PkgConfig::Systemd
    ${DtkCore_LIBRARIES}
)

# bin
install(TARGETS dde-display DESTINATION bin)
