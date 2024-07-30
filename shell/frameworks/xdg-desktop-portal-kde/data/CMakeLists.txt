configure_file(org.freedesktop.impl.portal.desktop.kde.desktop.in org.freedesktop.impl.portal.desktop.kde.desktop @ONLY)
configure_file(org.freedesktop.impl.portal.desktop.kde.cmake.in org.freedesktop.impl.portal.desktop.kde.service @ONLY)

install(FILES kde.portal DESTINATION ${KDE_INSTALL_DATADIR}/xdg-desktop-portal/portals)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.impl.portal.desktop.kde.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.impl.portal.desktop.kde.desktop DESTINATION ${KDE_INSTALL_APPDIR})

ecm_install_configured_files(INPUT plasma-xdg-desktop-portal-kde.service.in @ONLY
            DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
