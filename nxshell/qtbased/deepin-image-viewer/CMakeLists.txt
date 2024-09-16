cmake_minimum_required(VERSION 3.13)

project(deepin-image-viewer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_BINARY_DIR})
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

# Find the Qt library
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("Using Qt version: ${QT_VERSION_MAJOR}")

# Select major DTK version.
if(QT_VERSION_MAJOR EQUAL "6")
    set(BUILD_WITH_QT6 ON)
    set(DTK_VERSION_MAJOR 6)
else()
    set(DTK_VERSION_MAJOR "")
endif()

# Application
add_subdirectory(src)

add_subdirectory(qimage-plugins)

# Unit Tests
#add_subdirectory(tests)
