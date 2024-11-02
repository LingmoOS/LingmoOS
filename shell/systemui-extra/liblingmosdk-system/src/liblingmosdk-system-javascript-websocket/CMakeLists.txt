cmake_minimum_required(VERSION 3.13)
#project(liblingmosdk-system-javascript-httpserver-1.0)
project(liblingmosdk-system-javascript-websocket)
include(CMakePackageConfigHelpers)

include_directories("${PROJECT_BINARY_DIR}")

set(TARGET lingmosdk-dbus-websocket)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    lingmosdk/bios.cpp 
    lingmosdk/cpuinfo.cpp 
    lingmosdk/disk.cpp 
    lingmosdk/gps.cpp 
    lingmosdk/lingmosdkdbus.cpp 
    main.cpp 
    lingmosdk/mainboard.cpp 
    lingmosdk/net.cpp 
    lingmosdk/netcard.cpp 
    lingmosdk/netlink.cpp 
    lingmosdk/packageinfo.cpp 
    lingmosdk/peripheralsenum.cpp 
    lingmosdk/print.cpp 
    lingmosdk/process.cpp 
    lingmosdk/resolutionctl.cpp 
    lingmosdk/resource.cpp 
    lingmosdk/runinfo.cpp 
    lingmosdk/sysinfo.cpp 
    shared/websocketclientwrapper.cpp 
    shared/websockettransport.cpp 
    )
set(HEADERS 
    lingmosdk/bios.h 
    lingmosdk/cpuinfo.h 
    lingmosdk/dependence.h 
    lingmosdk/disk.h 
    lingmosdk/gps.h 
    lingmosdk/lingmosdkdbus.h 
    lingmosdk/mainboard.h 
    lingmosdk/net.h 
    lingmosdk/netcard.h 
    lingmosdk/netlink.h 
    lingmosdk/packageinfo.h 
    lingmosdk/peripheralsenum.h 
    lingmosdk/print.h 
    lingmosdk/process.h 
    lingmosdk/resolutionctl.h 
    lingmosdk/resource.h 
    lingmosdk/runinfo.h 
    lingmosdk/sysinfo.h 
    shared/websocketclientwrapper.h 
    shared/websockettransport.h 
    )
#set(FORMS lingmosdk/widget.ui)
#set(RESOURCES qrc.qrc)
set(QT Core Gui Widgets DBus Network Concurrent WebChannel WebSockets)
find_package(Qt5 REQUIRED ${QT})
add_executable(${TARGET} ${SOURCES} ${HEADERS} )
target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::Network
        Qt5::Concurrent
        Qt5::WebChannel
        Qt5::WebSockets
        )
set (CMAKE_INSTALL_PREFIX /usr)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})
