# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.13)

set(BIN_NAME "qdbus-service-demo")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(${BIN_NAME})

include(GNUInstallDirs)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
  find_package(deepin-qdbus-service REQUIRED) # just need this in your project
endif()

qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     org.deepin.service.sdk.demo.xml
                     service.h
                     Service)

file(GLOB_RECURSE SRCS
  "main.cpp"
  "service.cpp"
)

add_executable(${BIN_NAME}
    ${SRCS}
    ${ADAPTER_SOURCES}
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
    set(Service_INCLUDE_DIR ${SRC_BASE_DIR}/libqdbusservice)
    set(Service_LIBRARIES deepin-qdbus-service)
    target_link_directories(${BIN_NAME} PRIVATE
        ${CMAKE_BINARY_DIR}/lib
    )
else()
    # just need this in your project
    set(Service_INCLUDE_DIR ${deepin-qdbus-service_INCLUDE_DIR})
    set(Service_LIBRARIES ${deepin-qdbus-service_LIBRARIES})
    add_definitions(-DSERVICE_CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/deepin-service-manager/")
endif()

target_include_directories(${BIN_NAME} PUBLIC
  Qt5::Core
  Qt5::DBus
  ${Service_INCLUDE_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Core
  Qt5::DBus
  ${Service_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/other/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/sdk-demo.json ${CMAKE_BINARY_DIR}/share/other/)
else()
    # just need this in your project
    install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES sdk-demo.json DESTINATION share/deepin-service-manager/other/)
endif()

install(FILES org.deepin.service.sdk.demo.service DESTINATION share/dbus-1/system-services/)
install(FILES org.deepin.service.sdk.demo.conf
    DESTINATION share/dbus-1/system.d/
)
