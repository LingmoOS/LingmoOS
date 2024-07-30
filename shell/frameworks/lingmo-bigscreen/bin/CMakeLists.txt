configure_file(lingmo-bigscreen-wayland.in ${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-wayland)
install(PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-wayland
	lingmo-bigscreen-x11
	DESTINATION ${KDE_INSTALL_BINDIR})

configure_file(lingmo-bigscreen-wayland.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-wayland.desktop)
configure_file(lingmo-bigscreen-x11.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-x11.desktop)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-wayland.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/wayland-sessions
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/lingmo-bigscreen-x11.desktop
    DESTINATION ${KDE_INSTALL_DATADIR}/xsessions
)
