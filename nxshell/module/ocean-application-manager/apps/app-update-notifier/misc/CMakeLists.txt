# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
include(GNUInstallDirs)

configure_file(
    systemd/system/org.desktopspec.ApplicationUpdateNotifier1.service.in
    systemd/system/org.desktopspec.ApplicationUpdateNotifier1.service
    @ONLY
)

set(SYSTEMD_SYSTEM_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/systemd/system/org.desktopspec.ApplicationUpdateNotifier1.service
)

install(FILES ${SYSTEMD_SYSTEM_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/)

configure_file(
    dbus/org.desktopspec.ApplicationUpdateNotifier1.service.in
    dbus/org.desktopspec.ApplicationUpdateNotifier1.service
    @ONLY
)

set(DBUS_APPLICATION_UPDATE_NOTIFIER_SERVICE_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/dbus/org.desktopspec.ApplicationUpdateNotifier1.service
)

install(FILES ${DBUS_APPLICATION_UPDATE_NOTIFIER_SERVICE_FILE}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/dbus/org.desktopspec.ApplicationUpdateNotifier1.conf
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/system.d)
