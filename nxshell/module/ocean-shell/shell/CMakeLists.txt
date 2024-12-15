# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Widgets Gui WaylandClient)
find_package(TreelandProtocols REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED systemd)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

add_executable(ocean-shell
    main.cpp
    appletloader.h
    appletloader.cpp
    shell.h
    shell.cpp
    treelandoutputwatcher.h
    treelandoutputwatcher.cpp
    ocean-shell.qrc
    override_dtkdeclarative_qml.qrc
)

qt_generate_wayland_protocol_client_sources(ocean-shell
    FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-output-manager-v1.xml
)

target_compile_definitions(ocean-shell
PRIVATE
    DS_VERSION=${CMAKE_PROJECT_VERSION}
)

target_link_libraries(ocean-shell PRIVATE
    ocean-shell-frame
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WaylandClient
    Dtk${DTK_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    PkgConfig::WaylandClient
)

configure_file(${CMAKE_SOURCE_DIR}/misc/ocean-shell-plugin@.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-shell-plugin@.service
    @ONLY)
configure_file(${CMAKE_SOURCE_DIR}/misc/ocean-shell@.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-shell@.service
    @ONLY)

install(TARGETS ocean-shell DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ocean-shell@.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ocean-shell-plugin@.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
dtk_add_config_meta_files(APPID org.lingmo.ocean.shell FILES org.lingmo.ocean.shell.json)
