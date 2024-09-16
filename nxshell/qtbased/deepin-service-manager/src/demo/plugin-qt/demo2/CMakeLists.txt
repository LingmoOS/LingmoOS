# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.13)

set(BIN_NAME "plugin-qt-demo2")

project(${BIN_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(GNUInstallDirs)
find_package(Qt5 COMPONENTS Core Widgets DBus REQUIRED)

qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     org.deepin.service.demo2a.xml
                     service.h
                     Service)

# Sources files
file(GLOB_RECURSE SRCS "*.h" "*.cpp")



add_library(${BIN_NAME} MODULE
    ${SRCS}
    ${ADAPTER_SOURCES}
)

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
)


if (CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/user/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/plugin-qt-demo2.json ${CMAKE_BINARY_DIR}/share/user/)
else()
    # just need this in your project
    install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugin-qt-demo2.json DESTINATION share/deepin-service-manager/system/)
endif()

install(FILES org.deepin.service.demo2.conf
    DESTINATION share/dbus-1/system.d/
)
