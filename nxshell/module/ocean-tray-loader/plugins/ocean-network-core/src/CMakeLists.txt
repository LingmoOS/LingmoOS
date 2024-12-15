# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

project(ocean-tray-network-core)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options(-fPIC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

file(GLOB_RECURSE SOURCEFILES "*.h" "*.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCEFILES})

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS Core DBus Network Gui LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(KF6NetworkManagerQt REQUIRED)

pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)

file(GLOB TS_FILES "translations/*.ts")

set(TRY_KF6_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

foreach(LETTER ${TRY_KF6_LIBRARIES})
    if (NOT KF6_QT_LIBRARIES)
        get_target_property(KF6_QT_LIBRARIES KF6::NetworkManagerQt ${LETTER})
    endif()
endforeach()

if (NOT KF6_QT_LIBRARIES)
    find_library(KF6_QT_LIBRARIES libKF6NetworkManagerQt.so)
endif()
# end KF6 network manager

qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(networktranslations ALL DEPENDS ${QM_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    dbus
    impl
    impl/networkmanager
    impl/serviceinter
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Dtk${DTK_VERSION_MAJOR}::Core
    KF6::NetworkManagerQt
    PkgConfig::LibNM
)

message(STATUS ${LibNM_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Gui
    Dtk${DTK_VERSION_MAJOR}::Core
    KF6::NetworkManagerQt
    PkgConfig::LibNM
    udev
)

configure_file(ocean-tray-network-core.pc.in ${CMAKE_BINARY_DIR}/ocean-tray-network-core.pc @ONLY)

set(INTERFACEFILES
    impl/dslcontroller.h
    impl/hotspotcontroller.h
    impl/vpncontroller.h
    networkconst.h
    networkcontroller.h
    networkdetails.h
    networkdevicebase.h
    proxycontroller.h
    wireoceanvice.h
    wirelessdevice.h)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    # OUTPUT_NAME ocean-network-core
    # EXPORT_NAME Network-Core
)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/ocean-tray-network-core.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${INTERFACEFILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/liboceantraynetworkcore)
install(FILES ../config/org.lingmo.ocean.network.json DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.lingmo.ocean.tray.network) # compat
dtk_add_config_meta_files(APPID "org.lingmo.ocean.tray-loader" FILES ../config/org.lingmo.ocean.network.json)
# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
