# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.13)

set(BIN_NAME "network-service")

project(${BIN_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)
file(GLOB_RECURSE SRCS "*.h" "*.cpp")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-DQM_FILES_DIR="${CMAKE_BINARY_DIR}/network-service-plugin")
else()
    ADD_DEFINITIONS(-DQM_FILES_DIR="/usr/share/deepin-service-manager/network-service/translations")
endif()

find_package(Qt5 COMPONENTS Core Widgets DBus Network REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)
find_package(DtkCore REQUIRED)

file(GLOB TS_FILES "translations/*.ts")
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(${BIN_NAME}_language ALL DEPENDS ${QM_FILES})

pkg_check_modules(LibNM REQUIRED IMPORTED_TARGET libnm)


add_library(${BIN_NAME} MODULE
    ${SRCS}
)

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  ${Qt5Network_INCLUDE_DIRS}
  ${DFrameworkDBus_INCLUDE_DIRS}
  ${KF5_QT_INCLUDE_DIRS}
  ${DtkCore_INCLUDE_DIRS}
  .
  system
  session
  utils
  accountnetwork/system
  accountnetwork/system/accountnetwork
  accountnetwork/session/
  accountnetwork/session/accountnetwork
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
  ${Qt5Network_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${DFrameworkDBus_LIBRARIES}
  ${KF5_QT_LIBRARIES}
  ${DtkCore_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/system/)
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/user/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/system/plugin-system-network.json ${CMAKE_BINARY_DIR}/share/system/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/session/plugin-session-network.json ${CMAKE_BINARY_DIR}/share/user/)
else()
    # just need this in your project
    install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/system/plugin-system-network.json DESTINATION share/deepin-service-manager/system/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/session/plugin-session-network.json DESTINATION share/deepin-service-manager/user/)
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/system/org.deepin.service.SystemNetwork.conf DESTINATION share/dbus-1/system.d/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/session/org.deepin.service.SessionNetwork.conf DESTINATION share/dbus-1/system.d/)
# 安装.qm文件
install(FILES ${QM_FILES} DESTINATION share/deepin-service-manager/network-service/translations/)
