ecm_install_configured_files(INPUT lingmo-ksplash-ready.service.in @ONLY
                                                   DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})

install(FILES lingmo-core.target DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES lingmo-workspace.target DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES lingmo-workspace-wayland.target DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES lingmo-workspace-x11.target DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})

add_executable(kde-systemd-start-condition kde-systemd-start-condition.cpp)
target_link_libraries(kde-systemd-start-condition PUBLIC KF5::ConfigCore KF5::Service PlasmaAutostart)
install(TARGETS kde-systemd-start-condition DESTINATION ${KDE_INSTALL_BINDIR})
