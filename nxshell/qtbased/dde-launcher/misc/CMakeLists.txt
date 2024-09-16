# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

# dbus
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus/org.deepin.dde.Launcher1.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/dbus/org.deepin.dde.Launcher1.service
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dbus/org.deepin.dde.Launcher1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

# systemd
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/systemd/user/org.deepin.dde.Launcher1.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/systemd/user/org.deepin.dde.Launcher1.service
    @ONLY
)

set(SYSTEMD_USER_UNIT_DIR ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dde-session-initialized.target.wants)

file(CREATE_LINK
     ${SYSTEMD_USER_UNIT_DIR}/org.deepin.dde.Launcher1.service
     ${CMAKE_CURRENT_BINARY_DIR}/dde-session-initialized.target.wants/org.deepin.dde.Launcher1.service
     SYMBOLIC)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-session-initialized.target.wants/org.deepin.dde.Launcher1.service
        DESTINATION ${SYSTEMD_USER_UNIT_DIR}/dde-session-initialized.target.wants)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/systemd/user/org.deepin.dde.Launcher1.service
    DESTINATION ${SYSTEMD_USER_UNIT_DIR})
