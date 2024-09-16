# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.13)

set(Plugin_NAME "plugin-sdbus-demo1")

project(${Plugin_NAME})

find_package(PkgConfig REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

# Sources files
file(GLOB_RECURSE SRCS "*.h" "*.cpp")

add_library(${Plugin_NAME} MODULE
    ${SRCS}
)

target_include_directories(${Plugin_NAME} PUBLIC
    PkgConfig::Systemd
)

target_link_libraries(${Plugin_NAME}
    PkgConfig::Systemd
    -lpthread
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/user/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/plugin-sdbus-demo1.json ${CMAKE_BINARY_DIR}/share/user/)
else()
    # just need this in your project
    install(TARGETS ${Plugin_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugin-sdbus-demo1.json DESTINATION share/deepin-service-manager/user/)
endif()
