# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

configure_file(
    org.deepin.dde.daemon.Launcher1.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Launcher1.service
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Launcher1.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
