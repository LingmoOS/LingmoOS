# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0


# FIXME: copy from dde-network-core need to fix dde-network-core and remove those code
find_package(PkgConfig REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)
pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)
include_directories(${LibNM_INCLUDE_DIRS})

pkg_check_modules(Gio REQUIRED gio-2.0)
include_directories(${Gio_INCLUDE_DIRS})

pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

set(TRY_KF5_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF5_QT_LIBRARIES)
    find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
endif()

find_package(Qt5 COMPONENTS Core Widgets DBus Network LinguistTools REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)
get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

# test networkmanagetqt
try_compile(DEEPIN_NMQT_TEST
              ${CMAKE_CURRENT_BINARY_DIR}/try_compile
            SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/try-compile/networkmanagerqt.cc
            LINK_LIBRARIES
              ${KF5_QT_LIBRARIES}
              Qt5::Core
              Qt5::DBus
              Qt5::Network
            CMAKE_FLAGS
              "-DINCLUDE_DIRECTORIES=${KF5_QT_INCLUDE_DIRS}\;${GLIB_INCLUDE_DIRS}\;${LibNM_INCLUDE_DIRS}"
            OUTPUT_VARIABLE
              result
)
message(STATUS DEEPIN_NMQT_TEST ": ${DEEPIN_NMQT_TEST}")


add_subdirectory(application-tray)
add_subdirectory(dde-dock)
if (DEEPIN_NMQT_TEST)
    add_subdirectory(dde-network-core)
    add_subdirectory(dde-network-display-ui)
endif(DEEPIN_NMQT_TEST)
