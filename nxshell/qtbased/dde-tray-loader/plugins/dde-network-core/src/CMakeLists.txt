# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.7)

project(dde-tray-network-core)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
set(CMAKE_PREFIX_PATH $ENV{Qt5_DIR})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
endif()

file(GLOB_RECURSE SOURCEFILES "*.h" "*.cpp")

file(GLOB TS_FILES "translations/*.ts")

qt5_add_translation(QM_FILES ${TS_FILES})

add_custom_target(networktranslations ALL DEPENDS ${QM_FILES})

add_library(${PROJECT_NAME} STATIC ${SOURCEFILES})

find_package(Qt5 COMPONENTS Core DBus Network Gui REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)
pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(Libudev REQUIRED libudev)

# KF5 network manager
 get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

 set(TRY_KF5_LIBRARIES
 IMPORTED_LOCATION_DEBIAN
 IMPORTED_LOCATION_NOCONFIG
 )

 foreach(LETTER ${TRY_KF5_LIBRARIES})
     if (NOT KF5_QT_LIBRARIES)
         get_target_property(KF5_QT_LIBRARIES KF5::NetworkManagerQt ${LETTER})
     endif()
 endforeach()

 if (NOT KF5_QT_LIBRARIES)
     find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
 endif()
 # end KF5 network manager

target_include_directories(${PROJECT_NAME} PUBLIC
    dbus
    impl
    impl/networkmanager
    impl/serviceinter
    ${Qt5DBus_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${KF5_QT_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    PkgConfig::LibNM
    ${Libudev_INCLUDE_DIRS}
)

message(STATUS ${LibNM_INCLUDE_DIRS})
message(STATUS ${Libudev_INCLUDE_DIRS})
message(STATUS ${Libudev_LIBRARIES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Core_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${KF5_QT_LIBRARIES}
    ${DtkCore_LIBRARIES}
    PkgConfig::LibNM
    ${Libudev_LIBRARIES}
)

configure_file(dde-tray-network-core.pc.in ${CMAKE_BINARY_DIR}/dde-tray-network-core.pc @ONLY)

set(INTERFACEFILES
    impl/dslcontroller.h
    impl/hotspotcontroller.h
    impl/vpncontroller.h
    networkconst.h
    networkcontroller.h
    networkdetails.h
    networkdevicebase.h
    proxycontroller.h
    wireddevice.h
    wirelessdevice.h)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_BINARY_DIR}/dde-tray-network-core.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(FILES ${INTERFACEFILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libddetraynetworkcore)
install(FILES ../config/org.deepin.dde.network.json DESTINATION ${CMAKE_INSTALL_DATADIR}/dsg/configs/org.deepin.dde.tray.network) # compat
dtk_add_config_meta_files(APPID "org.deepin.dde.tray-loader" FILES ../config/org.deepin.dde.network.json)
# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
