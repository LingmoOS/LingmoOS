configure_file(ocean-session-manager.service.in
               ocean-session-manager.service
               @ONLY)
configure_file(ocean-session-exit-task.service.in
               ocean-session-exit-task.service
               @ONLY)
configure_file(ocean-session-shutdown.service.in
               ocean-session-shutdown.service
               @ONLY)

set(SERVICES
    ocean-session-initialized.target
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-manager.service
    ocean-session-manager.target
    ocean-session-pre.target
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-exit-task.service
    ${CMAKE_CURRENT_BINARY_DIR}/ocean-session-shutdown.service
    ocean-session-shutdown.target
    ocean-session.target
)
install(FILES ${SERVICES} DESTINATION lib/systemd/user/)

install(DIRECTORY DESTINATION lib/systemd/user/ocean-session-pre.target.wants/)
set(OCEAN_SESSION_PRE_WANTS
    ocean-session-pre.target.wants/ocean-session@x11.service
    ocean-session-pre.target.wants/ocean-display.service
)

# ocean-session-initialized.target.wants - TODO: 暂时先放这里，后续应该转移到对应服务
install(DIRECTORY DESTINATION lib/systemd/user/ocean-session-initialized.target.wants/)
set(OCEAN_SESSION_INITIALIZED_WANTS
    ocean-session-initialized.target.wants/ocean-shell@OCEAN.service
    ocean-session-initialized.target.wants/ocean-lock.service
    ocean-session-initialized.target.wants/ocean-polkit-agent.service
)

install(FILES ${OCEAN_SESSION_PRE_WANTS} DESTINATION lib/systemd/user/)
install(FILES ${OCEAN_SESSION_INITIALIZED_WANTS} DESTINATION lib/systemd/user/)
install_symlink(ocean-session@x11.service ocean-session-pre.target.wants)
install_symlink(ocean-display.service ocean-session-pre.target.wants)
install_symlink(ocean-shell@OCEAN.service ocean-session-initialized.target.wants)
install_symlink(ocean-lock.service ocean-session-initialized.target.wants)
install_symlink(ocean-polkit-agent.service ocean-session-initialized.target.wants)

install(FILES ${OCEAN_SESSION_DAEMON_WANTS} DESTINATION lib/systemd/user/)
install(DIRECTORY ocean-session-shutdown.target.d DESTINATION lib/systemd/user/)
