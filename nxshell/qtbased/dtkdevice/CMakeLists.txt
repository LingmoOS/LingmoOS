cmake_minimum_required(VERSION 3.10)

# If do't define version number, specify the version number
set(DTK_VERSION "1.0.3" CACHE STRING "define project version")

project(DtkDevice
    VERSION ${DTK_VERSION}
    DESCRIPTION "DTK library for device."
    HOMEPAGE_URL https://github.com/linuxdeepin/dtkdevice
    LANGUAGES CXX C
)

set(ENABLE_Qt6 CACHE BOOL OFF)
if (ENABLE_Qt6)
    set(DTK_VERSION_MAJOR 6)
    set(QT_VERSION_MAJOR 6)
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")
else()
    set(QT_VERSION_MAJOR 5)
    set(BUILD_EXAMPLES ON CACHE BOOL "Build examples")
endif()

set(LIB_NAME dtk${DTK_VERSION_MAJOR}device)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${PROJECT_VERSION_MAJOR}/DDevice")
set(LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set(MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "Install dir for qt pri files")
set(CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_VERSION_MAJOR}Device" CACHE STRING "Install dir for cmake config files")
set(PKGCONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE STRING "Install dir for pkgconfig pc files")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -Wl,--as-needed")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
  endif ()
  string(REPLACE "-O3" "-Ofast" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
endif()

# Find necessary packages
find_package(PkgConfig REQUIRED)
pkg_search_module(LibNl3 REQUIRED IMPORTED_TARGET libnl-3.0)
pkg_search_module(LibNl3Route REQUIRED IMPORTED_TARGET libnl-route-3.0)


find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_subdirectory(src)

if (BUILD_DOCS)
  add_subdirectory(docs)
endif ()

if(BUILD_EXAMPLES)
  message("===================================")
  message("You can build and run examples now ")
  message("===================================")
  add_subdirectory(examples)
endif()

if(BUILD_TESTING)
  message("==================================")
  message("       Now Testing is enabled     ")
  message("==================================")
  enable_testing()
  add_subdirectory(tests)
endif()

configure_package_config_file(misc/DtkDeviceConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeviceConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeviceConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeviceConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeviceConfigVersion.cmake
    DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
)

configure_file(misc/dtkdevice.pc.in ${LIB_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${PKGCONFIG_INSTALL_DIR})

set(DTK_MODULE dtkdevice)
configure_file(misc/qt_lib_dtkdevice.pri.in qt_lib_dtkdevice.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkdevice.pri DESTINATION ${MKSPECS_INSTALL_DIR})
