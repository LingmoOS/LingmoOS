# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0


# FIXME: copy from ocean-network-core need to fix ocean-network-core and remove those code
find_package(PkgConfig REQUIRED)
find_package(KF6NetworkManagerQt REQUIRED)
pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)
include_directories(${LibNM_INCLUDE_DIRS})

pkg_check_modules(Gio REQUIRED gio-2.0)
include_directories(${Gio_INCLUDE_DIRS})

pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

set(TRY_KF6_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF6_QT_LIBRARIES)
    find_library(KF6_QT_LIBRARIES libKF6NetworkManagerQt.so)
endif()

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS Core Widgets DBus Network LinguistTools REQUIRED)
get_target_property(KF6_QT_INCLUDE_DIRS KF6::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

# test networkmanagetqt
try_compile(LINGMO_NMQT_TEST
              ${CMAKE_CURRENT_BINARY_DIR}/try_compile
            SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/try-compile/networkmanagerqt.cc
            LINK_LIBRARIES
              KF6::NetworkManagerQt
              Qt${QT_VERSION_MAJOR}::Core
              Qt${QT_VERSION_MAJOR}::DBus
              Qt${QT_VERSION_MAJOR}::Network
            CMAKE_FLAGS
              "-DINCLUDE_DIRECTORIES=${KF6_QT_INCLUDE_DIRS}\;${GLIB_INCLUDE_DIRS}\;${LibNM_INCLUDE_DIRS}"
            OUTPUT_VARIABLE
              result
)
message(STATUS LINGMO_NMQT_TEST ": ${LINGMO_NMQT_TEST}")


add_subdirectory(application-tray)
add_subdirectory(ocean-dock)
add_subdirectory(libdbusmenuqt)
if (LINGMO_NMQT_TEST)
    add_subdirectory(ocean-network-core)
    add_subdirectory(ocean-network-display-ui)
endif(LINGMO_NMQT_TEST)
