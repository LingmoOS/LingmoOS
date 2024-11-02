cmake_minimum_required(VERSION 3.5)
project(bluetooth VERSION 1.0)  

find_package(PkgConfig)

cmake_minimum_required(VERSION 3.16)
# 查找Qt5及其模块
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core DBus)

set(CMAKE_AUTOMOC ON)

file(GLOB LIBBLUETOOTH_SOURCES src/*.cpp)

add_library(bluetooth STATIC ${LIBBLUETOOTH_SOURCES})  

target_link_libraries(bluetooth
    PRIVATE
    Qt5::Core
    Qt5::DBus)
