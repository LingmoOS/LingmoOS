# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED systemd)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

configure_file(
    org.deepin.dde.daemon.Launcher1.service.in
    ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Launcher1.service
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Launcher1.service
        DESTINATION ${SYSTEMD_USER_UNIT_DIR})
