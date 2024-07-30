## For Lingmo end users

if(LINGMO_X11_DEFAULT_SESSION)
   set(LINGMO_X11_DESKTOP_FILENAME lingmo.desktop CACHE INTERNAL "Filename for Lingmo X11 session")
   set(LINGMO_WAYLAND_DESKTOP_FILENAME lingmowayland.desktop CACHE INTERNAL "Filename for Lingmo Wayland session")
else()
   set(LINGMO_X11_DESKTOP_FILENAME lingmox11.desktop CACHE INTERNAL "Filename for Lingmo X11 session")
   set(LINGMO_WAYLAND_DESKTOP_FILENAME lingmo.desktop CACHE INTERNAL "Filename for Lingmo Wayland session")
endif()

configure_file(lingmox11.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/${LINGMO_X11_DESKTOP_FILENAME})
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${LINGMO_X11_DESKTOP_FILENAME}
        DESTINATION ${KDE_INSTALL_DATADIR}/xsessions
)

configure_file(lingmowayland.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/${LINGMO_WAYLAND_DESKTOP_FILENAME})
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${LINGMO_WAYLAND_DESKTOP_FILENAME}
        DESTINATION ${KDE_INSTALL_DATADIR}/wayland-sessions
)

## For Lingmo developers
configure_file(startlingmo-dev.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/startlingmo-dev.sh)
configure_file(lingmox11-dev.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmox11-dev6.desktop)
configure_file(lingmowayland-dev.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/lingmowayland-dev6.desktop)
configure_file(install-sessions.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/install-sessions.sh)
