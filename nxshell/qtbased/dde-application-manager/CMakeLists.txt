cmake_minimum_required(VERSION 3.20)
project(dde-application-manager
    VERSION 0.2.2
    DESCRIPTION "deepin application manager"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dde-application-manager"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

add_definitions(-Wall -Wextra -Wpedantic -Wformat)

set(BUILD_EXAMPLES OFF CACHE BOOL "Whether to build examples or not.")
set(DDE_AM_USE_DEBUG_DBUS_NAME OFF CACHE BOOL "build a dbus service using a different bus name for debug.")
set(PROFILING_MODE OFF CACHE BOOL "run a valgrind performance profiling.")

find_package(Qt6 REQUIRED COMPONENTS Core DBus Concurrent)
find_package(Dtk6 REQUIRED COMPONENTS Core)
find_package(Threads REQUIRED)

set(APP_LAUNCH_HELPER_BIN app-launch-helper)
set(APP_UPDATE_NOTIFIER_BIN app-update-notifier)
set(AM_LIBEXEC_DIR ${CMAKE_INSTALL_LIBEXECDIR}/deepin/application-manager)
set(APPLICATION_SERVICEID "org.deepin.dde.application-manager")

if(DDE_AM_USE_DEBUG_DBUS_NAME)
    add_compile_definitions(-DDDE_AM_USE_DEBUG_DBUS_NAME)
endif()

if(PROFILING_MODE)
    add_compile_definitions(-DPROFILING_MODE)
endif()

add_subdirectory(src)
add_subdirectory(plugins)
add_subdirectory(apps)
add_subdirectory(misc)
add_subdirectory(api)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
