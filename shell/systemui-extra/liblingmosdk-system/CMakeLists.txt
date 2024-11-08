cmake_minimum_required(VERSION 3.1.3)

project(lingmosdk-system)

set (CMAKE_SKIP_RPATH ON)

include(CMakePackageConfigHelpers)
include(FindPkgConfig)
include (GNUInstallDirs)

pkg_check_modules(PKGS REQUIRED
                dbus-1
                glib-2.0
                dbus-glib-1
                libudev
                )
include_directories(${PKGS_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}")
include_directories("/usr/include/lingmosdk/lingmosdk-base")
# link_directories("/usr/lib/lingmosdk/lingmosdk-base")
# link_directories("/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/")
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        link_directories("/usr/lib/arm-linux-gnueabihf/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        link_directories("/usr/lib/mips64el-linux-gnuabi64/")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        link_directories("/usr/lib/i386-linux-gnu/")
else()
        link_directories("/usr/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/")
endif()

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

message("Build Type: ${CMAKE_BUILD_TYPE}")

set (CMAKE_INSTALL_PREFIX /usr)
# set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/)
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "armv8l")
        set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/arm-linux-gnueabihf/)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/mips64el-linux-gnuabi64/)
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
        set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/i386-linux-gnu/)
else()
        set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib/${CMAKE_HOST_SYSTEM_PROCESSOR}-linux-gnu/)
endif()

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)
