cmake_minimum_required(VERSION 3.5)
find_package(PkgConfig)
project(lingmo-framework VERSION 1.0)


find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core DBus Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(dbus)

include_directories(dbus/devices/screen/src)  

add_executable(lingmo-framework
  main.cpp
)

target_link_libraries(lingmo-framework
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    screen)
