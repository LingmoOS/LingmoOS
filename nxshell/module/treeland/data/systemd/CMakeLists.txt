macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_INSTALL_PREFIX}/lib/systemd/user/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION lib/systemd/user/${wantsdir}/)
endmacro(install_symlink)

configure_file("ocean-session-pre.target.wants/treeland-sd.service.in" "ocean-session-pre.target.wants/treeland-sd.service" IMMEDIATE @ONLY)
configure_file("ocean-session-pre.target.wants/treeland-xwayland.service.in" "ocean-session-pre.target.wants/treeland-xwayland.service" IMMEDIATE @ONLY)
configure_file("ocean-session-pre.target.wants/treeland-shortcut.service.in" "ocean-session-pre.target.wants/treeland-shortcut.service" IMMEDIATE @ONLY)
configure_file("ocean-session-pre.target.wants/treeland.service.in" "ocean-session-pre.target.wants/treeland.service" IMMEDIATE @ONLY)

install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

install(DIRECTORY DESTINATION lib/systemd/user/ocean-session-pre.target.wants/)
set(OCEAN_SESSION_PRE_WANTS
    ocean-session-pre.target.wants/treeland-sd.socket
    ocean-session-pre.target.wants/treeland-xwayland.socket
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-pre.target.wants/treeland-sd.service
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-pre.target.wants/treeland-xwayland.service
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-pre.target.wants/treeland-shortcut.service
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-pre.target.wants/treeland.service
)

install(FILES ${OCEAN_SESSION_PRE_WANTS} DESTINATION lib/systemd/user/)
install_symlink(treeland-sd.socket ocean-session-pre.target.wants)
install_symlink(treeland-sd.service ocean-session-pre.target.wants)
install_symlink(treeland-xwayland.socket ocean-session-pre.target.wants)
install_symlink(treeland-xwayland.service ocean-session-pre.target.wants)
install_symlink(treeland-shortcut.service ocean-session-pre.target.wants)
install_symlink(treeland.service ocean-session-pre.target.wants)
