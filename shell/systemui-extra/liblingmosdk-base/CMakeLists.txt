cmake_minimum_required(VERSION 3.1.3)

project(lingmosdk-base)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

include_directories("${PROJECT_BINARY_DIR}")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_SKIP_RPATH ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("Build Type: ${CMAKE_BUILD_TYPE}")

set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set (CMAKE_INSTALL_PREFIX /usr)
# set (CMAKE_INSTALL_RPATH /usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set(CMAKE_INSTALL_RPATH "/usr/lib/arm-linux-gnueabihf/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set(CMAKE_INSTALL_RPATH "/usr/lib/mips64el-linux-gnuabi64/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set(CMAKE_INSTALL_RPATH "/usr/lib/i386-linux-gnu/")
else()
        set(CMAKE_INSTALL_RPATH "/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/")
endif()

add_subdirectory(src)
