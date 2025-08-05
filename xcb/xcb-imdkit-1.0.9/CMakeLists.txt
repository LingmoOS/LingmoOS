cmake_minimum_required(VERSION 3.8)
project(xcb-imdkit VERSION 1.0.9)

find_package(ECM 0.0.11 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
option(USE_SYSTEM_UTHASH "Enable system utash" On)

include(GenerateExportHeader)
include(CheckFunctionExists)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(uninstall)

if (USE_SYSTEM_UTHASH)
find_package(UTHash)
endif()

if (NOT TARGET UTHash::UTHash)
    add_library(UTHash::UTHash INTERFACE IMPORTED )
    set_target_properties(UTHash::UTHash PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/uthash"
    )
endif()

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

include(GNUInstallDirs)

# directory needed by bsd
if(NOT CMAKE_INSTALL_LIBDATADIR)
  set(CMAKE_INSTALL_LIBDATADIR "${CMAKE_INSTALL_LIBDIR}" CACHE PATH "libdata root (LIBDIR)")
  set(CMAKE_INSTALL_LIBDATADIR "${CMAKE_INSTALL_LIBDIR}")
  mark_as_advanced(CMAKE_INSTALL_LIBDATADIR)
endif()

set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/XCBImdkit")

add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "-fPIC -Wall -Wextra -Wno-unused-parameter -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined ${CMAKE_SHARED_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed ${CMAKE_MODULE_LINKER_FLAGS}")

find_package(XCB REQUIRED COMPONENTS XCB UTIL KEYSYMS)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

# Write out the features
feature_summary (WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
