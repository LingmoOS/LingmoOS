# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.13)

set(Plugin_NAME "plugin-oom")

project(${Plugin_NAME})

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)

# Sources files
file(GLOB_RECURSE SRCS "*.c" "*.h")

file(GLOB TS_FILES "translations/*.ts")
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(oomtranslations ALL DEPENDS ${QM_FILES})

add_library(${Plugin_NAME} MODULE
    ${SRCS}
)

target_include_directories(${Plugin_NAME} PUBLIC
    PkgConfig::Systemd
)

target_link_libraries(${Plugin_NAME}
    PkgConfig::Systemd
    pthread
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug") # delete this judgement in your project
    EXECUTE_PROCESS(COMMAND mkdir -p ${CMAKE_BINARY_DIR}/share/system/)
    EXECUTE_PROCESS(COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/plugin-oom.json ${CMAKE_BINARY_DIR}/share/system/)
    target_compile_definitions(${Plugin_NAME} PRIVATE _DEBUG)
else()
    # just need this in your project
    install(TARGETS ${Plugin_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/deepin-service-manager/)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plugin-oom.json DESTINATION share/deepin-service-manager/system/)
endif()

install(FILES org.deepin.oom1.conf
    DESTINATION share/dbus-1/system.d/
)

install(FILES org.deepin.oom.json
    DESTINATION share/dsg/configs/org.deepin.oom/
)

install(FILES earlyoom.service.json
    DESTINATION lib/deepin-daemon/service-trigger/)

# 安装.qm文件
install(FILES ${QM_FILES} DESTINATION share/deepin-service-manager/oom/translations)
# 安装getfromqm脚本
install(FILES getfromqm DESTINATION bin)
