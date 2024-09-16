# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
include(GNUInstallDirs)

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_symlink)

# # systemd service
configure_file(
    systemd/user/org.desktopspec.ApplicationManager1.service.in
    systemd/user/org.desktopspec.ApplicationManager1.service
    @ONLY
)

set(SYSTEMD_USER_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/systemd/user/org.desktopspec.ApplicationManager1.service
)

set(SYSTEMD_USER_DROP_IN_FILE
    ${CMAKE_CURRENT_SOURCE_DIR}/systemd/user/app-DDE-.service.d/override.conf
)

set(SERVICE_DEST_PATH ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/)

install(FILES ${SYSTEMD_USER_FILE} DESTINATION ${SERVICE_DEST_PATH})
install(FILES ${SYSTEMD_USER_DROP_IN_FILE} DESTINATION ${SERVICE_DEST_PATH}/app-DDE-.service.d)

# create a symlink to dde-session
install_symlink(org.desktopspec.ApplicationManager1.service dde-session-initialized.target.wants)

# # dbus activate
configure_file(
    dbus/org.desktopspec.ApplicationManager1.service.in
    dbus/org.desktopspec.ApplicationManager1.service
    @ONLY
)

set(DBUS_APPLICATION_MANAGER_SERVICE_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/dbus/org.desktopspec.ApplicationManager1.service
)

# install to session service directory for now
install(FILES ${DBUS_APPLICATION_MANAGER_SERVICE_FILE}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

# install dpkg hook
install(FILES ${CMAKE_CURRENT_LIST_DIR}/dpkg/dpkg.cfg.d/am-update-hook
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/dpkg/dpkg.cfg.d)

set(HOOKS_DEST_DIR ${CMAKE_INSTALL_DATADIR}/deepin/dde-application-manager/hooks.d)
configure_file(
    hooks.d/2-debFix.json.in
    hooks.d/2-debFix.json
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hooks.d/2-debFix.json
        DESTINATION ${HOOKS_DEST_DIR})

install(FILES ${CMAKE_CURRENT_LIST_DIR}/hooks.d/debFix.sh
        DESTINATION ${AM_LIBEXEC_DIR}
        PERMISSIONS
            OWNER_READ OWNER_EXECUTE OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
)

dtk_add_config_meta_files(APPID ${APPLICATION_SERVICEID}
    FILES
    ${CMAKE_CURRENT_LIST_DIR}/dsg/configs/dde-application-manager/org.deepin.dde.am.json
    ${CMAKE_CURRENT_LIST_DIR}/dsg/configs/dde-application-manager/org.deepin.dde.application-manager.json
)
