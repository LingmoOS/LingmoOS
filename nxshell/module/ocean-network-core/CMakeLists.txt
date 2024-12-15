cmake_minimum_required(VERSION 3.7)

set(VERSION "2.0.18" CACHE STRING "define project version")
project(ocean-network-core
  VERSION ${VERSION}
  DESCRIPTION "OCEAN Network Core"
  HOMEPAGE_URL "https://github.com/lingmoos/ocean-network-core"
  LANGUAGES CXX C
)

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now -z noexecstack -pie")

option(ENABLE_LINGMO_NMQT "enable nmqt patch on lingmo" OFF)

set(OCEAN-Network-Core_INCLUDE_DIRS  "${CMAKE_SOURCE_DIR}/src-old")
set(OCEAN-Network-Core_LIBRARIES  ocean-network-core)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)
include_directories(${LibNM_INCLUDE_DIRS})

pkg_check_modules(Gio REQUIRED gio-2.0)
include_directories(${Gio_INCLUDE_DIRS})

pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

if (QT_DEBUG)
    message(" KF5_QT_INCLUDE_DIRS : " ${KF5_QT_INCLUDE_DIRS})
    message(" GLIB_INCLUDE_DIRS :" ${GLIB_INCLUDE_DIRS})
    message(" CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS ${result})
endif()

if (ENABLE_LINGMO_NMQT)
    add_definitions(-DUSE_LINGMO_NMQT)
endif()

message(STATUS LINGMO_NMQT_TEST ": ${LINGMO_NMQT_TEST}")

set(INTERFACE_VERSION "2.0")
add_definitions(-DINTERFACE_VERSION="${INTERFACE_VERSION}")
message("Interface Version: " ${INTERFACE_VERSION})
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr")
endif()
include(GNUInstallDirs)

add_subdirectory("src")
add_subdirectory("src-old")
add_subdirectory("oceanui-network-plugin")
add_subdirectory("oceanui-network")
add_subdirectory("tests")
add_subdirectory("example")
#add_subdirectory("dock-network-plugin")
add_subdirectory("dss-network-plugin")
add_subdirectory("dss_example")
#add_subdirectory("dock-example")
