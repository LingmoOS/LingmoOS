# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets)
if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED systemd)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

add_executable(dde-shell
    main.cpp
    appletloader.h
    appletloader.cpp
    shell.h
    shell.cpp
    dde-shell.qrc
    override_dtkdeclarative_qml.qrc
)

target_compile_definitions(dde-shell
PRIVATE
    DS_VERSION=${CMAKE_PROJECT_VERSION}
)

target_link_libraries(dde-shell PRIVATE
    dde-shell-frame
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Gui
)

configure_file(${CMAKE_SOURCE_DIR}/misc/dde-shell-plugin@.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/dde-shell-plugin@.service
    @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/misc/dde-shell@.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/dde-shell@.service
    @ONLY)

install(TARGETS dde-shell DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-shell@.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-shell-plugin@.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
dtk_add_config_meta_files(APPID org.deepin.dde.shell FILES org.deepin.dde.shell.json)
