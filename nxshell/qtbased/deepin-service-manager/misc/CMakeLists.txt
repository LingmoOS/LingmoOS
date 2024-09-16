# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/system/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/system/${wantsdir}/)
endmacro(install_symlink)
macro(install_user_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_user_symlink)

install(FILES org.deepin.ServiceManager1.conf
    DESTINATION share/dbus-1/system.d/
)

set(SYSTEMD_SLICE "")
set(SYSTEMD_INSTALL "")
configure_file(deepin-service-group@.service.in deepin-service-group@.service.system @ONLY)
configure_file(deepin-service-manager.service.in deepin-service-manager.service.system @ONLY)
set(SYSTEMD_INSTALL "[Install]\nWantedBy=multi-user.target")
configure_file(deepin-service-plugin@.service.in deepin-service-plugin@.service.system @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-manager.service.system
    DESTINATION lib/systemd/system/
    RENAME deepin-service-manager.service
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-group@.service.system
    DESTINATION lib/systemd/system/
    RENAME deepin-service-group@.service
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-plugin@.service.system
    DESTINATION lib/systemd/system/
    RENAME deepin-service-plugin@.service
)

set(SYSTEMD_SLICE "Slice=session.slice")
set(SYSTEMD_INSTALL "")
configure_file(deepin-service-group@.service.in deepin-service-group@.service.user @ONLY)
configure_file(deepin-service-manager.service.in deepin-service-manager.service.user @ONLY)
set(SYSTEMD_INSTALL "[Install]\nWantedBy=default.target")
configure_file(deepin-service-plugin@.service.in deepin-service-plugin@.service.user @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-manager.service.user
    DESTINATION lib/systemd/user/
    RENAME deepin-service-manager.service
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-group@.service.user
    DESTINATION lib/systemd/user/
    RENAME deepin-service-group@.service
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-service-plugin@.service.user
    DESTINATION lib/systemd/user/
    RENAME deepin-service-plugin@.service
)

install_symlink(deepin-service-manager.service multi-user.target.wants)
install_user_symlink(deepin-service-manager.service default.target.wants)

set(LIB_NAME "deepin-qdbus-service")

# config pkgconfig file
configure_file(${LIB_NAME}.pc.in ${LIB_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# config cmake file
configure_file(${LIB_NAME}Config.cmake.in ${LIB_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${LIB_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME})
