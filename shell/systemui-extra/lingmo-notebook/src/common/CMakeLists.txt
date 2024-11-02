cmake_minimum_required(VERSION 3.5)

project(common)

include(${PROJECT_SOURCE_DIR}/../../cmake/common.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

file(GLOB SRC
    "*.cpp"
    "*.h"
    "*.hpp"
)

add_library(${PROJECT_NAME} STATIC
    ${SRC}
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::DBus)
target_link_qgsettings_libraries(${PROJECT_NAME})
target_link_lingmosdk_libraries(${PROJECT_NAME})
target_link_KWindowSystem_libraries(${PROJECT_NAME})