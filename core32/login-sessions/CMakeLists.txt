## For Plasma end users

if(PLASMA_WAYLAND_DEFAULT_SESSION)
   set(PLASMA_X11_DESKTOP_FILENAME lingmox11.desktop CACHE INTERNAL "Filename for Lingmo X11 session")
else()
   set(PLASMA_X11_DESKTOP_FILENAME plasma.desktop CACHE INTERNAL "Filename for Lingmo X11 session")
endif()

configure_file(lingmox11.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/${PLASMA_X11_DESKTOP_FILENAME})
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PLASMA_X11_DESKTOP_FILENAME}
        DESTINATION ${KDE_INSTALL_DATADIR}/xsessions
)

# configure_file(plasmawayland.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/${PLASMA_WAYLAND_DESKTOP_FILENAME})
# install(FILES
#         ${CMAKE_CURRENT_BINARY_DIR}/${PLASMA_WAYLAND_DESKTOP_FILENAME}
#         DESTINATION ${KDE_INSTALL_DATADIR}/wayland-sessions
# )

## For Plasma developers
configure_file(startlingmo-dev.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/startlingmo-dev.sh)
configure_file(lingmox11-dev.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmox11-dev.desktop)
configure_file(install-sessions.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/install-sessions.sh)
