cmake_minimum_required(VERSION 3.4)
project(QtZeroConfExample)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Gui Widgets REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find QtZeroConf library if we're building standalone
if(NOT TARGET QtZeroConf)
    find_package(QtZeroConf CONFIG REQUIRED)
endif()

add_executable(QtZeroConfExample
    window.h
    window.cpp
    main.cpp
)

target_link_libraries(QtZeroConfExample
    QtZeroConf
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
