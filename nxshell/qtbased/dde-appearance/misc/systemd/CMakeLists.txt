find_package(PkgConfig REQUIRED)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_USER_UNIT_DIR}/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION ${SYSTEMD_USER_UNIT_DIR}/${wantsdir}/)
endmacro(install_symlink)

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/dde-appearance.service.in
    dde-appearance.service
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-appearance.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

install_symlink(dde-appearance.service dde-session-initialized.target.wants)
