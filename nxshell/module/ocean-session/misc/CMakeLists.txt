configure_file(xsessions/ocean-x11.desktop.in xsessions/ocean-x11.desktop)

set(XSESSION
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/00lingmo-ocean-env
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/01lingmo-profile
)

set(PROFILE
    ${CMAKE_CURRENT_SOURCE_DIR}/profile.d/lingmo-xdg-dir.sh
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/xsessions
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}
    PATTERN
        "*.in" EXCLUDE
)

install(FILES ${XSESSION} DESTINATION /etc/X11/Xsession.d/)
install(FILES ${PROFILE} DESTINATION /etc/profile.d)
